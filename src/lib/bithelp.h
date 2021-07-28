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

#ifndef DW_BITHELP_H
#define DW_BITHELP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct bit_extractor {
  // 0x1CE3
  // Represents number of bits that are remaining to be read from bit_buffer.
  uint8_t num_bits;

  // 0x1CE5
  // Will contain actual remaining bits.
  uint8_t bit_buffer;

  unsigned char *data;
  uint16_t offset;
};

uint8_t bit_extract(struct bit_extractor *be, int n);

#ifdef __cplusplus
}
#endif

#endif /* DW_BITHELP_H */
