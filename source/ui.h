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

#ifndef DW_UI_H
#define DW_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct ui_header {
  int len; // 0x288A
  unsigned char data[16]; // 0x288B
};

struct ui_rect {
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
};

void ui_load();
void draw_viewport();
void ui_draw();
void ui_clean();

void ui_header_reset();
void ui_header_draw();
void ui_header_set_byte(unsigned char byte);

void ui_draw_box_segment(uint8_t chr, struct ui_rect *rect, struct ui_rect *outer);
void ui_draw_chr_piece(uint8_t chr, struct ui_rect *rect, struct ui_rect *outer);
void draw_pattern(struct ui_rect *rect);
void ui_set_background(uint16_t val);

#ifdef __cplusplus
}
#endif

#endif /* DW_UI_H */
