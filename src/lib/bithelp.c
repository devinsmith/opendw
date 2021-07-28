/*
 * Copyright (c) 2020-2021 Devin Smith <devin@devinsmith.net>
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

#include "bithelp.h"

// Extract "n" bits out of each byte.
// bit_buffer contains leftover bit buffer.
//
// 0x1D86 -> 1D8C(6)
// 0x1D8A -> 1D8C(5)
// 0x1D8C (num_bits passed in BL)
uint8_t bit_extract(struct bit_extractor *be, int n)
{
  int counter = n;
  int al = 0;
  int dl = be->num_bits;
  while (counter > 0) {
    dl--;
    if (dl < 0) {
      dl = be->data[be->offset];
      be->bit_buffer = dl;
      dl = 7;
      be->offset++;
    }
    // 0x1D96
    uint8_t tmp = be->bit_buffer;
    be->bit_buffer = be->bit_buffer << 1;

    // rcl al, 1
    int carry = 0;
    if (tmp > be->bit_buffer) {
      carry = 1;
    }
    al = al << 1;
    al += carry;
    counter--;
  }
  be->num_bits = dl; // leftover bits
  return al;
}

