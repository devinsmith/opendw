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

#ifndef __DW_RESOURCE_H__
#define __DW_RESOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Resource file maps to "data1" and "data2" files. */
enum resource_section {
  RESOURCE_UNKNOWN = 0x00,
  RESOURCE_TITLE0 = 0x18,
  RESOURCE_TITLE1 = 0x19,
  RESOURCE_TITLE2 = 0x1A,
  RESOURCE_TITLE3 = 0x1D
};

struct resource {
  unsigned char *bytes;
  size_t len;
};

int rm_init(void);
void rm_exit(void);

int resource_load(enum resource_section sec, struct resource *out);

unsigned char *com_extract(size_t off, size_t sz);

#ifdef __cplusplus
}
#endif

#endif /* __DW_RESOURCE_H__ */
