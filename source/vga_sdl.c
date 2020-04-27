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

#ifndef NO_DISPLAY
#include <SDL.h>
#endif

#include "vga.h"

#define WIN_WIDTH 640
#define WIN_HEIGHT 400
#define VGA_WIDTH 320
#define VGA_HEIGHT 200

#ifndef NO_DISPLAY
static SDL_Window *main_window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
#endif

/* http://www.brackeen.com/vga/basics.html */
const uint32_t vga_palette[] = {
  0x000000, /* BLACK */
  0x000080, /* BLUE */
  0x008000, /* GREEN */
  0x008080, /* CYAN */
  0x800000, /* RED */
  0x800080, /* MAGENTA */
  0x808000, /* BROWN */
  0xC0C0C0, /* LIGHT GRAY */
  0x808080, /* DARK GRAY */
  0x0000FF, /* LIGHT BLUE */
  0x00FF00, /* LIGHT GREEN */
  0x00FFFF, /* LIGHT CYAN */
  0xFF0000, /* LIGHT RED */
  0xFF00FF, /* LIGHT MAGENTA */
  0xFFFF00, /* YELLOW */
  0xFFFFFF, /* WHITE */
};

/* Represents 0xA0000 (0xA000:0000) memory, but in 32 bit for SDL */
uint32_t *framebuffer;

int
display_start(int game_width, int game_height)
{
#ifndef NO_DISPLAY
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

  if ((texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
    SDL_TEXTUREACCESS_STREAMING, game_width, game_height)) == NULL) {
    fprintf(stderr, "Main texture could not be created. SDL Error: %s\n",
      SDL_GetError());
    return -1;
  }
#endif

  if ((framebuffer = calloc(VGA_WIDTH * VGA_HEIGHT,
    sizeof(uint32_t))) == NULL) {
    fprintf(stderr, "Framebuffer could not be allocated.\n");
    return -1;
  }

  return 0;
}

void
display_end(void)
{
  free(framebuffer);

#ifndef NO_DISPLAY
  if (texture != NULL) {
    SDL_DestroyTexture(texture);
  }

  if (renderer != NULL) {
    SDL_DestroyRenderer(renderer);
  }

  if (main_window != NULL) {
    SDL_DestroyWindow(main_window);
  }
  SDL_Quit();
#endif
}

void
display_update(void)
{
#ifndef NO_DISPLAY
  SDL_UpdateTexture(texture, NULL, framebuffer, VGA_WIDTH * sizeof(uint32_t));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
#endif
}
