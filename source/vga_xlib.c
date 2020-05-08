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

Display *dpy;
int screen;
Window root;
XVisualInfo *vi;

int
display_start(int game_width, int game_height)
{
  const char *disp_env;
  XVisualInfo vinfo;
  int num_visuals;

  disp_env = getenv("DISPLAY");
  dpy = XOpenDisplay(disp_env);
  if (dpy == NULL) {
    fprintf(stderr, "Failed to open X display %s\n",
        disp_env == NULL ? "(null)" : disp_env);
    return -1;
  }
  screen = DefaultScreen(dpy);
  root = RootWindow(dpy, screen);

  vinfo.screen = screen;
  vinfo.depth = 8;
  vinfo.class = PseudoColor;

  vi = XGetVisualInfo(dpy, VisualScreenMask | VisualDepthMask |
      VisualClassMask, &vinfo, &num_visuals);

  if ((vi == NULL) || (num_visuals == 0)) {
    fprintf(stderr, "No visuals found!\n");
    return -1;
  }

  if (vi->class != PseudoColor) {
    fprintf(stderr, "Currently no support for non-TrueColor visuals.\n");
    return -1;
  }




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
  return NULL;
}

struct vga_driver x_driver = {
  "xlib",
  display_start,
  display_end,
  display_update,
  waitkey,
  get_fb_mem
};

struct vga_driver *vga = &x_driver;
