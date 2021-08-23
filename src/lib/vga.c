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

#include <stdlib.h>
#include <stdio.h>

#include "vga.h"

#ifndef nitems
#define nitems(_a) (sizeof((_a)) / sizeof((_a)[0]))
#endif /* nitems */

static struct vga_driver *vga = NULL;

#define VGA_WIDTH 320
#define VGA_HEIGHT 200

/* Represents 0xA0000 (0xA000:0000) memory. */
static uint8_t *framebuffer;

static struct key_buffer {
  int buffer[32];
  int head;
  int tail;
  int count;
} vga_keyb;

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

static uint8_t *
get_fb_mem()
{
  return framebuffer;
}

void register_vga_driver(struct vga_driver *driver)
{
  vga = driver;
}

int vga_initialize(int game_width, int game_height)
{
  vga_keyb.head = 0;
  vga_keyb.tail = 0;
  vga_keyb.count = 0;

  if (vga != NULL && vga->initialize != NULL) {
    return vga->initialize(game_width, game_height);
  }
  return display_start(game_width, game_height);
}

uint8_t* vga_memory()
{
  if (vga != NULL && vga->memory != NULL) {
    return vga->memory();
  }
  return get_fb_mem();
}

void vga_update()
{
  if (vga != NULL && vga->update != NULL) {
    vga->update();
  }
}

uint16_t vga_getkey()
{
  if (vga != NULL && vga->getkey != NULL) {
    return vga->getkey();
  }
  return 0;
}

void vga_waitkey()
{
  if (vga != NULL && vga->waitkey != NULL) {
    vga->waitkey();
  }
}

void vga_end()
{
  if (vga != NULL && vga->end != NULL) {
    vga->end();
  } else {
    display_end();
  }
}

void vga_poll_events()
{
  if (vga != NULL && vga->poll != NULL) {
    vga->poll();
  }
}

void vga_addkey(int key)
{
  if (vga_keyb.count == nitems(vga_keyb.buffer)) {
    return;
  }

  vga_keyb.buffer[vga_keyb.head] = key;
  vga_keyb.head = (vga_keyb.head + 1) % nitems(vga_keyb.buffer);
  vga_keyb.count++;
}

int vga_getkey2()
{
  int key;

  if (vga_keyb.count == 0) {
    return 0;
  }

  key = vga_keyb.buffer[vga_keyb.tail];
  vga_keyb.count--;

  vga_keyb.tail = (vga_keyb.tail + 1) % nitems(vga_keyb.buffer);
  return key;
}
