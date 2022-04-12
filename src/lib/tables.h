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

#ifndef DW_TABLES_H
#define DW_TABLES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void load_chr_table();
void unload_chr_table();
const unsigned char *get_chr(int chr_num);
uint8_t get_b152_table(uint8_t offset);
uint8_t get_and_table(uint8_t offset);
uint8_t get_or_table(uint8_t offset);
uint8_t get_1BC1_table(uint8_t offset);
uint16_t get_line_offset(uint8_t offset);
uint16_t get_unknown_4456(uint8_t index);
uint16_t get_and_table_B452(uint8_t offset);
uint16_t get_or_table_B652(uint8_t offset);
uint16_t get_ba52_data(uint8_t offset);

#ifdef __cplusplus
}
#endif

#endif /* DW_TABLES_H */
