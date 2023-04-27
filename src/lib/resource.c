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
#include <resource.h>
#include "player.h"
#include "ui.h"

/* Only deals with data1 */
/* I'm not sure yet how data2 is used */

// 0xBC52
static unsigned char data1_hdr[768];
static struct buf_rdr *header_rdr = NULL;

static struct resource allocations[128] = { 0 };
static struct resource *resource_load_cache_miss(enum resource_section sec);

static unsigned char *ptr3; // 0x313E

#ifndef nitems
#define nitems(_a) (sizeof((_a)) / sizeof((_a)[0]))
#endif /* nitems */

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

/* 0x1348 */
struct resource* game_memory_alloc(size_t nbytes, int marker, int tag)
{
  int i;
  struct resource *a;

  for (i = 0; i < nitems(allocations); i++) {
    a = &allocations[i];
    if (a->usage_type == 0) {
      break;
    }
  }

  if (i == nitems(allocations))
    return NULL;

  a = &allocations[i];
  a->bytes = malloc(nbytes);
  if (a->bytes == NULL)
    return NULL;

  a->usage_type = marker;
  a->len = nbytes;
  a->tag = tag;

  return a;
}

/* 0x12C0 inside dragon.com
 * XXX: Rename this. */
int find_index_by_tag(int tag)
{
  int i;
  for (i = 0; i < nitems(allocations); i++) {
    struct resource *a = &allocations[i];
    if (a->tag == tag && a->usage_type != 0) {
      return i;
    }
  }

  return -1;
}

// 0x1CF
int
rm_init(void)
{
  /* First two entries are some unknown data in the COM file, I think,
   * but I'm not sure how they are used.
   * For now we just load unknown_data and hope they aren't used. */
  allocations[0].bytes = get_player_data_base();
  allocations[0].tag = 0xFFFF;
  allocations[0].usage_type = 0xFF;
  allocations[0].len = 1;

  allocations[1].bytes = get_player_data_base();
  allocations[1].tag = 0xFFFF;
  allocations[1].usage_type = 0xFF;
  allocations[1].len = 0x0E00;

  for (int i = 0; i < nitems(allocations); i++) {
    allocations[i].index = i;
  }

  // first allocation will be saved at 0x02.
  return load_data1_header();
}

void
rm_exit(void)
{
  if (header_rdr != NULL) {
    buf_rdr_free(header_rdr);
  }

  // Clean up resource cache.
  for (int i = 0; i < nitems(allocations); i++) {
    if (allocations[i].bytes != NULL && allocations[i].usage_type == 1) {
      free(allocations[i].bytes);
    }
  }
}

// Essentially 0x2EB0 but not exactly.
// AL is always loaded with the marker character (always 1?)
// BX contains the section to load.
struct resource* resource_load(enum resource_section sec)
{
  if (sec >= RESOURCE_MAX) {
    fprintf(stderr, "Attempted to load unknown resource, returning NULL\n");
    return NULL;
  }

  // Check cache.
  int index = find_index_by_tag(sec);
  if (index != -1) {
    return &allocations[index];
  }

  // Not found.
  return resource_load_cache_miss(sec);
}

// 0x12A8
struct resource* resource_get_by_index(int index)
{
  return &allocations[index];
}

// 0x1270
void resource_index_release(int index)
{
  if (index == 0xFF)
    return;

  if (index < 2)
    return;

  allocations[index].usage_type = 0;
  free(allocations[index].bytes);
  allocations[index].bytes = NULL;
}

// 0x1297
void resource_set_usage_type(int index, int usage_type)
{
  if (index == 0xFF)
    return;

  if (index < 2)
    return;

  allocations[index].usage_type = usage_type;
}

// 0x128D
void resource_set_flagged(int index)
{
  // We still don't really know what usage_type 0x2 means, so we're
  // going with "flagged" for now.
  resource_set_usage_type(index, 0x2);
}

static struct resource *
resource_load_cache_miss(enum resource_section sec)
{
  int i;
  unsigned int offset = sizeof(data1_hdr);
  FILE *fp;
  size_t n;
  uint16_t len;
  int needs_decompression = 0;

  buf_reset(header_rdr);
  for (i = 0; i < sec; i++) {
    uint16_t header_val = buf_get16le(header_rdr);
    if (header_val < 0xFF00) {
      offset += header_val;
    }
  }
  len = buf_get16le(header_rdr);
  printf("Section (0x%02x), Offset: 0x%04x Size: 0x%04x\n", sec, offset,
    (unsigned int)len);

  struct resource* res = game_memory_alloc(len, 1, sec);
  if (sec > 0x17) {
    needs_decompression = 1;
  }

  fp = fopen("data1", "rb");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open data1 file.\n");
    return NULL;
  }

  fseek(fp, offset, SEEK_SET);
  n = fread(res->bytes, 1, res->len, fp);
  if (n != res->len) {
    fprintf(stderr, "Failed to read section in data1 file.\n");
    free(res->bytes);
    free(res);
    fclose(fp);
    return NULL;
  }
  fclose(fp);

  if (needs_decompression) {
    struct buf_rdr *compression_rdr;
    struct buf_wri *uncompressed_wri;
    unsigned short uncompressed_sz;

    compression_rdr = buf_rdr_init(res->bytes, res->len);
    uncompressed_sz = buf_get16le(compression_rdr);
    printf("Section 0x%02X needs decompression. %d -> %d\n", sec,
        (unsigned int)res->len, uncompressed_sz);
    uncompressed_wri = buf_wri_init(uncompressed_sz);
    decompress_data1(compression_rdr, uncompressed_wri, uncompressed_sz);

    // After decompressing we can get rid of the compressed copy, and store the
    // data directly in the resource.
    free(res->bytes);
    res->len = uncompressed_sz;
    // move pointer.
    res->bytes = uncompressed_wri->base;
    free(uncompressed_wri);
    buf_rdr_free(compression_rdr);
  }

  return res;
}

/* The DOS COM executable format sets the origin by default at 0x100. So when
 * extracting from COM files we subtract this from the offset.
 *
 * Quite a few assets/resources are actually embedded within the DRAGON.COM
 * file, so we extract them with this function. */

#define COM_ORG_START 0x100
unsigned char *com_extract(size_t off, size_t sz)
{
  unsigned char *ptr;
  FILE *fp;

  if (off < COM_ORG_START) {
    fprintf(stderr, "com_extract: Invalid offset specified, too low!\n");
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
    fprintf(stderr, "com_extract: Failed to read %zu bytes from file.\n", sz);
    free(ptr);
    return NULL;
  }

  fclose(fp);
  return ptr;
}

// 0x0A25
void setup_memory()
{
  init_viewport_memory();
  ptr3 = malloc(0x370 * 16);
}
