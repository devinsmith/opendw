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

#ifndef DW_RESOURCE_H
#define DW_RESOURCE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Resource file maps to "data1" and "data2" files. */
enum resource_section {
  RESOURCE_SCRIPT = 0x00,
  RESOURCE_CHARACTER_DATA = 0x07,
  RESOURCE_TITLE0 = 0x18,
  RESOURCE_TITLE1 = 0x19,
  RESOURCE_TITLE2 = 0x1A,
  RESOURCE_TITLE3 = 0x1D,
  RESOURCE_UNKNOWN = 0x47,
  RESOURCE_LAST = 0xFF,
  RESOURCE_MAX
};

struct resource {
  unsigned char *bytes;
  size_t len;
  // Seems to be one of these values:
  // 0x00 - Not used, free spot.
  // 0x01 - dynamically allocated, must be free'd
  // ...  - ??? (not sure if values other than 0, 1, and 0xFF are used)
  // 0xFF - Statically allocated
  int usage_type;
  int tag;
  int index;
};

int rm_init(void);
void rm_exit(void);

struct resource* resource_get_by_index(int index);
void resource_index_release(int index);
void resource_set_usage_type(int index, int usage_type);

// 0x2EB0
struct resource* resource_load(enum resource_section sec);

int find_index_by_tag(int tag);
unsigned char *com_extract(size_t off, size_t sz);
struct resource* game_memory_alloc(size_t nbytes, int marker, int tag);
void setup_memory();

#ifdef __cplusplus
}
#endif

#endif /* __DW_RESOURCE_H__ */
