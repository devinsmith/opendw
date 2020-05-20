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

/* Represents 0xA0000 (0xA000:0000) memory. */
static uint8_t *framebuffer;

static int
display_start(int game_width, int game_height)
{
  if ((framebuffer = calloc(VGA_WIDTH * VGA_HEIGHT, 1)) == NULL) {
    fprintf(stderr, "Framebuffer could not be allocated.\n");
    return -1;
  }

  return 0;
}

static void
display_end(void)
{
  free(framebuffer);
}

static void
display_update(void)
{
}

static void waitkey()
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

struct vga_driver null_driver = {
  "null",
  display_start,
  display_end,
  display_update,
  waitkey,
  get_fb_mem,
  get_key
};

struct vga_driver *vga = &null_driver;
