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

#include <bufio.h>
#include <display.h>
#include <resource.h>

static void
run_title(void)
{
  struct resource title_res;
  struct buf_rdr *title_rdr;
  struct buf_wri *title_wri;
  unsigned int uncompressed_sz;

  if (resource_load(RESOURCE_TITLE, &title_res) != 0)
    return;

  title_rdr = buf_rdr_init(title_res.bytes, title_res.len);

  uncompressed_sz = buf_get16le(title_rdr);
  printf("Unc: 0x%04x\n", uncompressed_sz);
  title_wri = buf_wri_init(uncompressed_sz);

  buf_wri_free(title_wri);
  buf_rdr_free(title_rdr);
  free(title_res.bytes);
}

int
main(int argc, char *argv[])
{
  /* XXX: We need to do a pre-flight check and make sure all the files
   * that we are going to use are in place. Maybe RM can do it. */

  if (rm_init() != 0) {
    goto done;
  }

  if (display_start() != 0) {
    goto done;
  }

  run_title();

done:
  rm_exit();
  display_end();
  return 0;
}
