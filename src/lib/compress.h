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

#ifndef __COMPRESS_H__
#define __COMPRESS_H__

#include <bufio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Container for extracting bits from a buffer.
struct bit_extractor {
    // 0x1CE3
    // Represents number of bits that are remaining to be read from bit_buffer.
    uint8_t num_bits;

    // 0x1CE4
    // Uppercase flag
    uint8_t upper_case;

    // 0x1CE5
    // Will contain actual remaining bits.
    uint8_t bit_buffer;

    const unsigned char *data;
    uint16_t offset;
};

/*!***************************************************************************
 * @short  Extract n number of bits from bit_extractor.
 * @param  be  Bit extractor to extract bits from.
 * @param  n   Number of bits to extract, n must be <= 8.
 * @return Extracted bit value.
 *****************************************************************************/
uint8_t bit_extract(struct bit_extractor *be, int n);

/*!***************************************************************************
 * @short  Extracts a single letter from a bit_extractor container.
 * @param  be  Bit extractor to extract letter from.
 * @return Extracted letter.
 *****************************************************************************/
uint8_t extract_letter(struct bit_extractor *be);

/*!***************************************************************************
 * @short  Decompress data1 content.
 * @param  input  Buffered reader to read from.
 * @param  output Buffered writer to write to.
 * @param  size   number of bytes expected to decompress to.
 * @return
 *****************************************************************************/
void decompress_data1(struct buf_rdr *input, struct buf_wri *output, int size);

#ifdef __cplusplus
}
#endif

#endif /* __COMPRESS_H__ */
