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

#include <SDL.h>

#include "vga.h"

#define WIN_WIDTH 640
#define WIN_HEIGHT 400
#define VGA_WIDTH 320
#define VGA_HEIGHT 200

static SDL_Window *main_window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Surface *surface = NULL;

// https://github.com/canidlogic/vgapal
static const SDL_Color sdl_palette[] = {
  { 0x00, 0x00, 0x00, 0xFF }, /* BLACK */
  { 0x00, 0x00, 0xAA, 0xFF }, /* BLUE */
  { 0x00, 0xAA, 0x00, 0xFF }, /* GREEN */
  { 0x00, 0xAA, 0xAA, 0xFF }, /* CYAN */
  { 0xAA, 0x00, 0x00, 0xFF }, /* RED */
  { 0xAA, 0x00, 0xAA, 0xFF }, /* MAGENTA */
  { 0xAA, 0x55, 0x00, 0xFF }, /* BROWN */
  { 0xAA, 0xAA, 0xAA, 0xFF }, /* LIGHT GRAY */
  { 0x55, 0x55, 0x55, 0xFF }, /* DARK GRAY */
  { 0x55, 0x55, 0xFF, 0xFF }, /* LIGHT BLUE */
  { 0x55, 0xFF, 0x55, 0xFF }, /* LIGHT GREEN */
  { 0x55, 0xFF, 0xFF, 0xFF }, /* LIGHT CYAN */
  { 0xFF, 0x55, 0x55, 0xFF }, /* LIGHT RED */
  { 0xFF, 0x55, 0xFF, 0xFF }, /* LIGHT MAGENTA */
  { 0xFF, 0xFF, 0x55, 0xFF }, /* YELLOW */
  { 0xFF, 0xFF, 0xFF, 0xFF } /* WHITE */
};

static const uint16_t normal_scancodes[] = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x1E61, 0x3062, // ....ab
  0x2E63, 0x2064, 0x1265, 0x2166, 0x2267, 0x2368, // cedfgh
  0x1769, 0x246A, 0x256B, 0x266C, 0x326D, 0x316E, // ijklmn
  0x186F, 0x1970, 0x1071, 0x1372, 0x1F73, 0x1474, // opqrst
  0x1675, 0x2F76, 0x1177, 0x2D78, 0x1579, 0x2C7A, // uvwxyz
  0x0231, 0x0332, 0x0433, 0x0534, 0x0635, 0x0736, // 123456
  0x0837, 0x0938, 0x0A39, 0x0B40, 0x1C0D, 0x011B, // 7890<Enter><Esc>
  0x0E08, 0x0F09, 0x3920, 0x0C2D, 0x0D3D, 0x1A5B, // <Backspace><Tab><Space>-=[
  0x1B5D, 0x2B5C, 0x0000, 0x273B, 0x2827, 0x2960, // ]\.;'`
  0x332C, 0x342E, 0x352F, 0x0000, 0x0000, 0x0000, // ,./...
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // F3-F8
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // F9-ScrollLock
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // Pause-End
  0x0000, 0x4D00, 0x4B00, 0x5000, 0x4800          // Arrow keys
};

int
display_start(int game_width, int game_height)
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "SDL could not initialize. SDL Error: %s\n",
      SDL_GetError());
    return -1;
  }

  if ((main_window = SDL_CreateWindow("OpenDW",
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    WIN_WIDTH, WIN_HEIGHT,
    SDL_WINDOW_RESIZABLE)) == NULL) {
    fprintf(stderr, "Main window could not be created. SDL Error: %s\n",
      SDL_GetError());
    return -1;
  }

  if ((renderer = SDL_CreateRenderer(main_window, -1, 0)) == NULL) {
    fprintf(stderr, "Main renderer could not be created. SDL Error: %s\n",
      SDL_GetError());
    return -1;
  }

  if ((surface = SDL_CreateRGBSurface(SDL_SWSURFACE, game_width, game_height,
       8 /* bpp */, /* RGBA masks */ 0, 0, 0, 0)) == NULL) {
    fprintf(stderr, "8 bit surface could not be created. SDL Error: %s\n",
      SDL_GetError());
    return -1;
  }

  if (SDL_SetPaletteColors(surface->format->palette, sdl_palette, 0, 16) != 0) {
    fprintf(stderr, "Failed to set palette. SDL Error: %s\n",
      SDL_GetError());
    return -1;
  }

  return 0;
}

void
display_end(void)
{
  if (surface != NULL) {
    SDL_FreeSurface(surface);
  }

  if (renderer != NULL) {
    SDL_DestroyRenderer(renderer);
  }

  if (main_window != NULL) {
    SDL_DestroyWindow(main_window);
  }
  SDL_Quit();
}

void
display_update(void)
{
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
  SDL_DestroyTexture(texture);
}

void waitkey()
{
  SDL_Event event;
  int done = 0;

  while (!done) {

    SDL_WaitEvent(&event);
    switch (event.type) {
    case SDL_QUIT:
    case SDL_KEYDOWN:
      done = 1;
      break;
    }
  }
}

static uint8_t *
get_fb_mem()
{
  return surface->pixels;
}

static uint16_t
shifted(const SDL_Keysym *key)
{
  if (key->sym >= 'a' && key->sym <= 'z') {
    return key->sym - 0x20;
  }

  // Unhandled shift key.
  return key->sym;
}

static uint16_t
get_key()
{
  SDL_Event e;


  while (SDL_WaitEventTimeout(&e, 2000)) {
    if (e.type == SDL_KEYDOWN) {
      const SDL_KeyboardEvent *ke = &e.key;
      const SDL_Keysym *ksym = &ke->keysym;

      // Under DOS we capture both the key code and the scan code.
      // in AX (AH/AL). Certain keys, like Ctrl, arrow keys, etc
      // have an AL value of 0x00
      //
      // 0x2E7B (arrow keys)
      if (ksym->sym == SDLK_LEFT)
        return 0x88;
      if (ksym->sym == SDLK_RIGHT)
        return 0x95;
      if (ksym->sym == SDLK_DOWN)
        return 0x8A;
      if (ksym->sym == SDLK_UP)
        return 0x8B;

      // Add special cases.
      // Special case to capture + key on some keyboards.
      if ((ksym->sym == SDLK_EQUALS && ksym->mod & KMOD_SHIFT) ||
          ksym->sym == SDLK_PLUS) {
        return 0x2B | 0x80;
      }

      // Special case to capture ? key.
      if ((ksym->sym == SDLK_SLASH && ksym->mod & KMOD_SHIFT) ||
          ksym->sym == SDLK_QUESTION) {
        printf("question mark ?\n");
        return 0x3F | 0x80;
      }

      // Normal keys.
      if ((ksym->sym & SDLK_SCANCODE_MASK) == 0) {
        printf("sym: 0x%08X\n", ksym->sym);
        printf("mod: 0x%04X\n", ksym->mod);
        printf("scan: 0x%04X\n", ksym->scancode);
        if (ksym->mod & KMOD_CTRL) {
          if (ksym->sym == SDLK_s) {
            return 0x13 | 0x80;
          }
        }
        if (ksym->mod & KMOD_SHIFT) {
          return shifted(ksym) | 0x80;
        }
        return (uint8_t)ksym->sym | 0x80;
      }
    }
  }
  return 0;
}

static void poll_events()
{

}

struct vga_driver sdl_driver = {
  "SDL", // 2.0
  display_start,
  display_end,
  display_update,
  waitkey,
  get_fb_mem,
  get_key,
  poll_events
};

void video_setup()
{
  register_vga_driver(&sdl_driver);
}
