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

#include "engine.h"
#include "offsets.h"
#include "resource.h"
#include "state.h"
#include "tables.h"
#include "utils.h"
#include "ui.h"
#include "vga.h"

/* Original Dragon Wars resoluation */
#define GAME_WIDTH 320
#define GAME_HEIGHT 200

static void
title_adjust(const struct resource *title)
{
  unsigned char *src, *dst;
  int i, counter = 0x3E30;
  uint16_t ax, si;

  src = title->bytes;
  dst = title->bytes + 0xA0;

  for (i = 0; i < counter; i++) {
    ax = *src++;
    ax += *(src++) << 8;

    si = *(src + 0x9E);
    si += *(src + 0x9F) << 8;

    ax = ax ^ si;

    /* write output_idx (16 bits) to output in little endian format. */
    *(dst++) = (ax & 0xff);
    *(dst++) = (ax & 0xff00) >> 8;
  }
}

static void
title_build(const struct resource *output)
{
  uint8_t al;
  unsigned char *src = output->bytes;
  int i, counter = 64000;
  int hi, lo;
  uint8_t *framebuffer = vga->memory();

  for (i = 0; i < counter; i += 2) {

    al = *src++;

    /* Each nibble represents a color */
    /* for example 0x82 represents color 8 then color 2. */
    hi = (al >> 4) & 0xf;
    lo = al & 0xf;

    framebuffer[i] = hi;
    framebuffer[i + 1] = lo;
  }
}

/* 0x387 */
static void
run_title(void)
{
  const struct resource *title_res = resource_load(RESOURCE_TITLE3);
  if (title_res == NULL)
    return;

  title_adjust(title_res);

  dump_hex(title_res->bytes, 32);
  title_build(title_res);

  vga->update();
  resource_index_release(title_res->index);

  vga->waitkey();
}

int check_file(const char *fname)
{
  FILE *fp = fopen(fname, "rb");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open %s. Can't proceed.\n", fname);
    return 0;
  }
  fclose(fp);
  return 1;
}

int
check_files(void)
{
  /* We only check for the existance of data1, data2, and dragon.com.
   * We can't do signature checks on these files yet because they are actually
   * modified by the game (yes even dragon.com writes data back into itself).
   */

  return check_file("dragon.com") &&
    check_file("data1") &&
    check_file("data2");
}

int
main(int argc, char *argv[])
{
  if (check_files() == 0) {
    return -1;
  }

  if (rm_init() != 0) {
    goto done;
  }

  setup_memory();

  init_offsets();
  load_chr_table();

  byte_4F0F = 0xFF;
  set_game_state("main", 87, 0xFF);
  set_game_state("main", 91, 0xFF);
  set_game_state("main", 86, 0xFF);
  set_game_state("main", 90, 0xFF);
  set_game_state("main", 8, 0xFF);
  byte_4F10 = 0xFF;

  // Not sure where this is done or where it goes.
  // Indicates that part of the UI is drawn?
  for (int i = 24; i < 31; i++) {
    set_game_state("main", i, 0xFF);
  }

  if (vga->initialize(GAME_WIDTH, GAME_HEIGHT) != 0) {
    goto done;
  }

  ui_set_background(0);
  run_title();
  ui_load();
  sub_37C8();

  draw_rect.x = 1;
  draw_rect.y = 8;
  draw_rect.w = 39;
  draw_rect.h = 184;
  ui_drawn_yet = 0xFF;

  ui_draw_full();

  run_engine();

  ui_clean();

done:
  unload_chr_table();
  rm_exit();
  vga->end();
  return 0;
}
