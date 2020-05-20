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

#include <SDL.h>

#include "vga.h"

#define WIN_WIDTH 640
#define WIN_HEIGHT 400
#define VGA_WIDTH 320
#define VGA_HEIGHT 200

static SDL_Window *main_window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Surface *surface = NULL;

/* http://www.brackeen.com/vga/basics.html */
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
      done = 1;
      break;
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
get_key()
{
  SDL_Event e;

  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_KEYDOWN) {
      const SDL_KeyboardEvent *ke = &e.key;
      const SDL_Keysym *ksym = &ke->keysym;

      return ksym->sym;
    }
  }
  return 0;
}

struct vga_driver sdl_driver = {
  "SDL", // 2.0
  display_start,
  display_end,
  display_update,
  waitkey,
  get_fb_mem,
  get_key
};

struct vga_driver *vga = &sdl_driver;
