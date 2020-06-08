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

// data_320C
struct ui_string_line {
  int len;
  unsigned char bytes[40];
};

// 0x320C
extern struct ui_string_line ui_string;

struct ui_rect {
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
};

struct ui_point {
  uint16_t x;
  uint16_t y;
};

extern uint8_t ui_drawn_yet; // 0x268E
// 2697
extern struct ui_rect draw_rect;
// 32BF
extern struct ui_point draw_point;

void ui_load();
void draw_viewport();
void ui_draw();
void ui_draw_full();
void ui_clean();

void ui_header_reset();
void ui_header_draw();
void ui_header_set_byte(unsigned char byte);

void ui_draw_box_segment(uint8_t chr);
void ui_draw_chr_piece(uint8_t chr, struct ui_point *pt, struct ui_rect *outer);
void draw_pattern(struct ui_rect *rect);
void ui_set_background(uint16_t val);
void ui_draw_string(void);

void ui_rect_expand();
void ui_rect_shrink();

int ui_adjust_rect(uint8_t input);

void ui_set_byte_3236(uint8_t val);
uint8_t ui_get_byte_3236();

#ifdef __cplusplus
}
#endif

#endif /* DW_UI_H */
