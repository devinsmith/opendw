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

#include <stdint.h>

#include "offsets.h"

#define NUM_OFFSETS 0x88

// 0xB042
uint16_t offsets[NUM_OFFSETS];

// 0x1053 (always gets set to 0x50 ?)
unsigned short word_1053 = 0;
unsigned short word_1055 = 0;

// 0x17DD
void init_offsets(unsigned short dx)
{
  int i;
  uint16_t val = 0;

  // 0x17E5
  word_1053 = dx;

  // 0x17E5
  for (i = 0; i < NUM_OFFSETS; i++) {
    offsets[i] = val;
    val += 0x50;
  }
}

uint16_t get_offset(int pos)
{
  return offsets[pos];
}
