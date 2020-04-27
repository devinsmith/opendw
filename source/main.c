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

#include "bufio.h"
#include "compress.h"
#include "engine.h"
#include "offsets.h"
#include "resource.h"
#include "tables.h"
#include "utils.h"
#include "ui.h"
#include "vga.h"

/* Original Dragon Wars resoluation */
#define GAME_WIDTH 320
#define GAME_HEIGHT 200

static void
title_adjust(struct buf_wri *title)
{
  unsigned char *src, *dst;
  int i, counter = 0x3E30;
  uint16_t ax, si;

  src = title->base;
  dst = title->base + 0xA0;

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
title_build(struct buf_wri *output)
{
  uint8_t al;
  unsigned char *src = output->base;
  int i, counter = 64000;
  int hi, lo;

  for (i = 0; i < counter; i += 2) {

    al = *src++;

    /* Each nibble represents a color */
    /* for example 0x82 represents color 8 then color 2. */
    hi = (al >> 4) & 0xf;
    lo = al & 0xf;

    framebuffer[i] = vga_palette[hi];
    framebuffer[i + 1] = vga_palette[lo];
  }
}

/* 0x387 */
static void
run_title(void)
{
  struct buf_rdr *title_rdr;
  struct buf_wri *title_wri;
  unsigned int uncompressed_sz;

  const struct resource *title_res = resource_load(RESOURCE_TITLE3);
  if (title_res == NULL)
    return;

  title_rdr = buf_rdr_init(title_res->bytes, title_res->len);

  uncompressed_sz = buf_get16le(title_rdr);
  printf("Unc: 0x%04x\n", uncompressed_sz);
  title_wri = buf_wri_init(uncompressed_sz);

  /* decompress title data from title reader into title writer */
  decompress_data1(title_rdr, title_wri, uncompressed_sz);
  title_adjust(title_wri);

  dump_hex(title_wri->base, 32);
  title_build(title_wri);

  display_update();
  resource_index_release(title_res->index);

  waitkey();

  buf_wri_free(title_wri);
  buf_rdr_free(title_rdr);
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

  init_offsets();
  load_chr_table();

  set_game_state(87, 0xFF);
  set_game_state(91, 0xFF);
  set_game_state(86, 0xFF);
  set_game_state(90, 0xFF);
  set_game_state(8, 0xFF);

  // Not sure where this is done or where it goes.
  // Indicates that part of the UI is drawn?
  for (int i = 24; i < 31; i++) {
    set_game_state(i, 0xFF);
  }

  if (display_start(GAME_WIDTH, GAME_HEIGHT) != 0) {
    goto done;
  }

  run_title();
  ui_load();
  draw_viewport();
  ui_draw();

#ifndef NO_DISPLAY
  // Wait for key, temporary.
  waitkey();
#endif

  run_engine();

#ifndef NO_DISPLAY
  waitkey();
#endif

  ui_clean();

done:
  unload_chr_table();
  rm_exit();
  display_end();
  return 0;
}
