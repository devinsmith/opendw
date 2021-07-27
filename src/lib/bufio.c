/*
 * Copyright (c) 2014-2015 Devin Smith <devin@devinsmith.net>
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
#include <string.h>

#include <bufio.h>

/* Buffer writer implementation */
struct buf_wri *
buf_wri_init(size_t len)
{
  struct buf_wri *w;

  w = malloc(sizeof(struct buf_wri));
  if (w == NULL)
    return NULL;

  w->base = malloc(len);
  if (w->base == NULL) {
    free(w);
    return NULL;
  }
  w->len = 0;

  return w;
}

void
buf_wri_free(struct buf_wri *w)
{
  if (w != NULL) {
    free(w->base);
    free(w);
  }
}

void
buf_add8(struct buf_wri *w, uint8_t val)
{
  w->base[w->len++] = val;
}

/* Buffer reader implementation */
struct buf_rdr *
buf_rdr_init(unsigned char *data, size_t len)
{
  struct buf_rdr *r;

  r = malloc(sizeof(struct buf_rdr));
  if (r == NULL) return NULL;

  r->data = data;
  r->len = len;
  r->offset = 0;

  return r;
}

void
buf_reset(struct buf_rdr *r)
{
  r->offset = 0;
}

uint8_t
buf_get8(struct buf_rdr *r)
{
  return r->data[r->offset++];
}

uint16_t
buf_get16le(struct buf_rdr *r)
{
  uint16_t ret;

  ret = r->data[r->offset++];
  ret += r->data[r->offset++] << 8;

  return ret;
}

uint16_t
buf_get16be(struct buf_rdr *r)
{
  uint16_t ret;

  ret = r->data[r->offset++] << 8;
  ret += r->data[r->offset++];

  return ret;
}

uint32_t
buf_get32le(struct buf_rdr *r)
{
  uint32_t ret;

  ret = r->data[r->offset++];
  ret += r->data[r->offset++] << 8;
  ret += r->data[r->offset++] << 16;
  ret += r->data[r->offset++] << 24;

  return ret;
}

uint32_t
buf_get32be(struct buf_rdr *r)
{
  uint32_t ret;

  ret = r->data[r->offset++] << 24;
  ret += r->data[r->offset++] << 16;
  ret += r->data[r->offset++] << 8;
  ret += r->data[r->offset++];

  return ret;
}

void
buf_rdr_free(struct buf_rdr *r)
{
  if (r == NULL) return;
  free(r);
}
