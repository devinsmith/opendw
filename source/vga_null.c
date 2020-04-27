/*
 * Copyright (c) 2018-2020 Devin Smith <devin@devinsmith.net>
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

#include "vga.h"

#define VGA_WIDTH 320
#define VGA_HEIGHT 200

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
}

void
display_update(void)
{
}

void waitkey()
{
}
