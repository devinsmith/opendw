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

#include <stddef.h>

static struct mem_allocation {
  unsigned char *data;
  size_t bytes_allocated;
  int in_use;
  int tag;
} allocations[128] = { 0 };

#ifndef nitems
#define nitems(_a) (sizeof((_a)) / sizeof((_a)[0]))
#endif /* nitems */

/* 0x12C0 inside dragon.com */
int find_index_by_tag(int tag)
{
  int i = 0;
  for (i = 0; i < nitems(allocations); i++) {
    struct mem_allocation *a = &allocations[i];
    if (a->tag == tag)
    {
      break;
    }
  }

  if (i == nitems(allocations))
    return -1;

  // XXX: Not done.
  return i;
}

/* 0x1348 */
void *game_memory_alloc(size_t bytes)
{
  return NULL;
}
