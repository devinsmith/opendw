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

static struct resource* resource_cache[128];

#define COM_ORG_START 0x100

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

  // Clean up resource cache.
  for (int i = 0; i < 128; i++) {
    if (resource_cache[i] != NULL && resource_cache[i]->bytes != NULL) {
      free(resource_cache[i]->bytes);
      free(resource_cache[i]);
    }
  }
}

int resource_load_index(enum resource_section sec)
{
  if (sec >= RESOURCE_MAX) {
    return -1;
  }

  int index = 0;
  for (index = 0; index < 128; index++) {
    if (resource_cache[index] == NULL) {
      struct resource *cache_miss = malloc(sizeof(struct resource));
      resource_load(sec, cache_miss);
      resource_cache[index] = cache_miss;
      return index;
    }
  }
  return -1;
}

struct resource* resource_get_index(int index)
{
  return resource_cache[index];
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

/* The DOS COM executable format sets the origin by default at 0x100. So when
 * extracting from COM files we subtract this from the offset.
 *
 * Quite a few assets/resources are actually embedded within the DRAGON.COM
 * file, so we extract them with this function. */
unsigned char *com_extract(size_t off, size_t sz)
{
  unsigned char *ptr;
  FILE *fp;

  if (off < COM_ORG_START) {
    fprintf(stderr, "Invalid offset specified, too low!\n");
    return NULL;
  }
  off -= COM_ORG_START;

  fp = fopen("dragon.com", "rb");
  if (fp == NULL)
    return NULL;

  ptr = malloc(sz);
  if (ptr == NULL)
    return NULL;

  fseek(fp, off, SEEK_SET);
  if (fread(ptr, 1, sz, fp) != sz) {
    free(ptr);
    return NULL;
  }

  fclose(fp);
  return ptr;
}
