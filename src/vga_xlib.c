/*
 * Copyright (c) 2018 Devin Smith <devin@devinsmith.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "vga.h"

#define WIN_WIDTH 640
#define WIN_HEIGHT 400
#define VGA_WIDTH 320
#define VGA_HEIGHT 200

/* http://www.brackeen.com/vga/basics.html */
#if 0
static const SDL_Color sdl_palette[] = {
  { 0x00, 0x00, 0x00, 0xFF }, /* BLACK */
  { 0x00, 0x00, 0x80, 0xFF }, /* BLUE */
  { 0x00, 0x80, 0x00, 0xFF }, /* GREEN */
  { 0x00, 0x80, 0x80, 0xFF }, /* CYAN */
  { 0x80, 0x00, 0x00, 0xFF }, /* RED */
  { 0x80, 0x00, 0x80, 0xFF }, /* MAGENTA */
  { 0x80, 0x80, 0x00, 0xFF }, /* BROWN */
  { 0xC0, 0xC0, 0xC0, 0xFF }, /* LIGHT GRAY */
  { 0x80, 0x80, 0x80, 0xFF }, /* DARK GRAY */
  { 0x00, 0x00, 0xFF, 0xFF }, /* LIGHT BLUE */
  { 0x00, 0xFF, 0x00, 0xFF }, /* LIGHT GREEN */
  { 0x00, 0xFF, 0xFF, 0xFF }, /* LIGHT CYAN */
  { 0xFF, 0x00, 0x00, 0xFF }, /* LIGHT RED */
  { 0xFF, 0x00, 0xFF, 0xFF }, /* LIGHT MAGENTA */
  { 0xFF, 0xFF, 0x00, 0xFF }, /* YELLOW */
  { 0xFF, 0xFF, 0xFF, 0xFF } /* WHITE */
};
#endif

static Display *dpy;
static int screen, index_mode;
static Window root, win;
static XVisualInfo *vi;
static Colormap cmap;
static XColor clr[256];
static GC gc;
static uint8_t *framebuffer;
static uint8_t *imagebuffer;
static XImage *img;

static int detect_visual(int depth, int class)
{
  XVisualInfo vinfo;
  int i, num_visuals;

  vinfo.screen = screen;
  vinfo.depth = depth;
  vinfo.class = class;

  vi = XGetVisualInfo(dpy, VisualScreenMask | VisualDepthMask |
      VisualClassMask, &vinfo, &num_visuals);
  if (vi && (num_visuals > 0)) {
    index_mode = depth == 8 ? 1 : 0;

    printf("%d: rm:%lu gm:%lu bm:%lu cs:%d bpr:%d\n", depth, vi->red_mask,
        vi->green_mask, vi->blue_mask, vi->colormap_size, vi->bits_per_rgb);

    cmap = XCreateColormap(dpy, root, vi->visual, AllocAll);
    if (index_mode == 1) {
      for (i = 0; i < 256; i++) {
        clr[i].pixel = i;
        clr[i].flags = DoRed | DoGreen | DoBlue;
      }
    }

    return 1;
  }
  return 0;
}

static int setup_visual_colormap()
{
  return detect_visual(8, PseudoColor) ||
    detect_visual(15, TrueColor) ||
    detect_visual(16, TrueColor) ||
    detect_visual(24, TrueColor) ||
    detect_visual(32, TrueColor);
}

int
display_start(int game_width, int game_height)
{
  const char *disp_env;
  XSetWindowAttributes attr;
  XGCValues gcvalues;
  XSizeHints sizehints;
  int attrmask;
  int bpp;

  disp_env = getenv("DISPLAY");
  dpy = XOpenDisplay(disp_env);
  if (dpy == NULL) {
    fprintf(stderr, "Failed to open X display %s\n", XDisplayName(disp_env));
    return -1;
  }
  screen = DefaultScreen(dpy);
  root = RootWindow(dpy, screen);

  if (setup_visual_colormap() == 0) {
    fprintf(stderr, "No usable visual found!\n");
    return -1;
  }

  attr.colormap = cmap;
  attr.event_mask = KeyPressMask | KeyReleaseMask | ExposureMask;
  attrmask = CWColormap | CWEventMask;
  win = XCreateWindow(dpy, root, 0, 0, 320, 200, 0, CopyFromParent,
      InputOutput, vi->visual, attrmask, &attr);
  if (win == None) {
    fprintf(stderr, "Could not create window!\n");
    return -1;
  }

  gcvalues.foreground = BlackPixel(dpy, screen);
  gcvalues.background = WhitePixel(dpy, screen);
  gc = XCreateGC(dpy, win, GCForeground | GCBackground, &gcvalues);

  sizehints.min_width = 320;
  sizehints.min_height = 200;
  sizehints.max_width = 320;
  sizehints.max_height = 200;
  sizehints.base_width = 320;
  sizehints.base_height = 200;
  sizehints.flags = PMinSize | PMaxSize | PBaseSize;

  //XSetWMProperties(dpy, win, NULL, NULL, NULL, 0, &sizehints, None, None);
  XStoreName(dpy, win, "OpenDW");
  XSetIconName(dpy, win, "OpenDW");

  switch (vi->depth) {
  case 15:
  case 16:
    bpp = 2;
    break;
  case 24:
    bpp = 3;
    break;
  case 32:
    bpp = 4;
    break;
  default:
    bpp = 1;
    break;
  }

  if ((framebuffer = calloc(VGA_WIDTH * VGA_HEIGHT, 1)) == NULL) {
    fprintf(stderr, "Framebuffer could not be allocated.\n");
    return -1;
  }

  if (index_mode) {
    imagebuffer = framebuffer;
  } else {
    if ((imagebuffer = calloc(VGA_WIDTH * VGA_HEIGHT, bpp)) == NULL) {
      fprintf(stderr, "Image buffer could not be allocated.\n");
      return -1;
    }
  }

  img = XCreateImage(dpy, vi->visual, vi->depth, ZPixmap, 0,
      (char *)imagebuffer, 320, 200, 8, 320 * bpp);
  if (img == NULL) {
    fprintf(stderr, "Could not create X11 pixmap image.\n");
    return -1;
  }

  XMapWindow(dpy, win);

  return 0;
}

void
display_end(void)
{
}

void
display_update(void)
{
}

void waitkey()
{
}

static uint8_t *
get_fb_mem()
{
  return framebuffer;
}

static uint16_t
get_key()
{
  return 0;
}

struct vga_driver x_driver = {
  "xlib",
  display_start,
  display_end,
  display_update,
  waitkey,
  get_fb_mem,
  get_key
};

struct vga_driver *vga = &x_driver;
