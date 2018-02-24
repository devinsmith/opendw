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
#include <stdbool.h>

#include <bufio.h>
#include <resource.h>

/* Only deals with data1 */
/* I'm not sure yet how data2 is used */
static unsigned char data1_hdr[768];
static struct buf_rdr *header_rdr = NULL;

static int
load_data1_header(void)
{
  FILE *fp;
  size_t n;
  int rc = -1;

  fp = fopen("data1", "rb");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open data1 file.\n");
    return -1;
  }

  n = fread(data1_hdr, 1, sizeof(data1_hdr), fp);
  if (n != sizeof(data1_hdr)) {
    fprintf(stderr, "Failed to read data1 header bytes.\n");
    goto done;
  }

  if ((header_rdr = buf_rdr_init(data1_hdr, sizeof(data1_hdr))) == NULL) {
    fprintf(stderr, "Failed to create header reader.\n");
    goto done;
  }

  rc = 0;
done:
  fclose(fp);
  return rc;
}

int
rm_init(void)
{
  return load_data1_header();
}

void
rm_exit(void)
{
  if (header_rdr != NULL) {
    buf_rdr_free(header_rdr);
  }
}

int
resource_load(enum resource_section sec, struct resource *res)
{
  int i;
  unsigned int offset = sizeof(data1_hdr);
  FILE *fp;
  size_t n;

  if (res == NULL)
    return -1;

  buf_reset(header_rdr);
  for (i = 0; i < sec; i++) {
    uint16_t header_val = buf_get16le(header_rdr);
    if (header_val < 0xFF00) {
      offset += header_val;
    }
  }
  res->len = buf_get16le(header_rdr);
  printf("Section (0x%02x), Offset: 0x%04x Size: 0x%04x\n", sec, offset,
    (unsigned int)res->len);

  fp = fopen("data1", "rb");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open data1 file.\n");
    return -1;
  }

  fseek(fp, offset, SEEK_SET);
  res->bytes = malloc(res->len);
  if (res->bytes == NULL) {
    fclose(fp);
    return -1;
  }

  n = fread(res->bytes, 1, res->len, fp);
  if (n != res->len) {
    free(res->bytes);
    fclose(fp);
    return -1;
  }
  fclose(fp);

  return 0;
}

