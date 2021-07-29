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

#ifndef DW_ENGINE_H
#define DW_ENGINE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char byte_104E;
extern unsigned char byte_2476;
extern unsigned char data_2AAA[32];
extern unsigned char word_4C31[4];
extern unsigned char byte_4F0F;
extern unsigned char byte_4F10;

void reset_game_state();
void run_engine();
void sub_4D82();

uint16_t extract_string(const unsigned char *src_ptr, uint16_t offset, void (*func)(unsigned char));

#ifdef __cplusplus
}
#endif

#endif /* DW_ENGINE_H */
