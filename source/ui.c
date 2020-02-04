/*
 * Copyright (c) 2020 Devin Smith <devin@devinsmith.net>
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

#include "display.h"
#include "offsets.h"
#include "resource.h"
#include "tables.h"
#include "ui.h"
#include "utils.h"

struct viewport_data {
  int xpos;
  int ypos;
  int runlength;
  int numruns;
  int unknown1;
  int unknown2;
  unsigned char *data;
};

struct pic_data {
  int runlenth;
  int numruns;
  int unknown1;
  int unknown2;
  unsigned char *data;
};

static int loaded = 0;

struct viewport_data viewports[] = {
  {
    0x00, 0x00, 0x04, 0x0A, 0x00, 0x00,
    NULL
  },
  {
    0x98, 0x00, 0x04, 0x0A, 0x00, 0x00,
    NULL
  },
  {
    0x00, 0x7B, 0x04, 0x0D, 0x00, 0x00,
    NULL
  },
  {
    0x98, 0x7B, 0x04, 0x0D, 0x00, 0x00,
    NULL
  }
};

struct pic_data bottom_bricks = {
  0xA0, 0x10, 0x00, 0xB8, NULL
};

/* D88 */
static void process_quadrant(const struct viewport_data *d, unsigned char *data)
{
  int newx, newy;
  uint16_t offset;

  newx = d->xpos >> 1;
  newy = d->ypos << 1;
  printf("%02x %02x, %02x, %02x\n", d->xpos, d->ypos, newx, newy);
  offset = get_offset(d->ypos);
  offset += newx;
  printf("Offset: %04x\n", offset);
  unsigned char *p = data + offset;
  unsigned char *q = d->data;
  for (int i = 0; i < d->numruns; i++) {
    for (int j = 0; j < d->runlength; j++) {
      unsigned char val = *q;
      unsigned char dval = *p;

      dval = dval & get_and_table(val);
      dval = dval | get_or_table(val);

      printf("(%02x %02x) ", dval, val);
      *p = dval;
      p++;
      q++;
    }
    offset += 0x50;
    p = data + offset;
    printf("\n");
  }
}

void draw_viewport()
{
  int rows = 0x88;
  int cols = 0x50;
  int dest = 0x10;

  unsigned char *data = calloc(sizeof(unsigned char), rows * cols);

  /* Iterate backwards like DW does */
  int vidx = 3;
  while (vidx >= 0) {
    const struct viewport_data *p = &viewports[vidx];
    process_quadrant(p, data);
    vidx--;
  }

  // 0x88 x 0x50
  unsigned char *src = data;
  for (int y = 0; y < rows; y++) {
    uint16_t fb_off = get_drawing_offset(dest) + 0x10;
    for (int x = 0; x < cols; x++) {
      uint8_t al = *src++;
      int hi, lo;

      /* Each nibble represents a color */
      /* for example 0x82 represents color 8 then color 2. */
      hi = (al >> 4) & 0xf;
      lo = al & 0xf;

      framebuffer[fb_off++] = vga_palette[hi];
      framebuffer[fb_off++] = vga_palette[lo];
    }
    dest++;
  }
  display_update();

  free(data);
}

void draw_something()
{
  uint16_t offset = bottom_bricks.unknown2;
  printf("%04x\n", offset);
  uint16_t starting_off = get_drawing_offset(offset);
  uint16_t fb_off = starting_off;
  printf("%04x\n", fb_off);
  unsigned char *src = bottom_bricks.data;
  for (int y = 0; y < 0x10; y++) {
    for (int x = 0; x < 0xa0; x++) {
      uint8_t al = *src++;
      int hi, lo;

      /* Each nibble represents a color */
      /* for example 0x82 represents color 8 then color 2. */
      hi = (al >> 4) & 0xf;
      lo = al & 0xf;

      framebuffer[fb_off++] = vga_palette[hi];
      framebuffer[fb_off++] = vga_palette[lo];
    }
    starting_off += 0x140;
    fb_off = starting_off;
  }
  display_update();
}

void ui_load()
{
  /* Viewport data is stored in the dragon.com file */
  viewports[0].data = com_extract(0x665c, 4 * 0xA);
  viewports[1].data = com_extract(0x6688, 4 * 0xA);
  viewports[2].data = com_extract(0x66B4, 4 * 0xD);
  viewports[3].data = com_extract(0x66EC, 4 * 0xD);

  bottom_bricks.data = com_extract(0x6A3A, 0xA00);
  loaded = 1;
}

void ui_clean()
{
  free(viewports[0].data);
  free(viewports[1].data);
  free(viewports[2].data);
  free(viewports[3].data);

  free(bottom_bricks.data);
}
