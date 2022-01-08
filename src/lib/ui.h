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
#include "resource.h"

// data_320C
struct ui_string_line {
  int len;
  unsigned char bytes[40];
};

struct viewport_data {
  uint16_t xpos; // Sometimes set as 36C0
  int ypos; // Sometimes set as 36C4
  int runlength; // Sometimes set as 1048
  int numruns;
  int unknown1;
  int unknown2;
  unsigned char *data;
};

// 0x320C
extern struct ui_string_line ui_string;

struct ui_rect {
  uint16_t x; // 0x2697
  uint16_t y; // 0x2699
  uint16_t w; // 0x269B
  uint16_t h; // 0x269D
};

struct ui_point {
  uint16_t x; // 0x32BF
  uint16_t y; // 0x32C1
};

extern uint8_t ui_drawn_yet; // 0x268E
// 2697
extern struct ui_rect draw_rect;
// 32BF
extern struct ui_point draw_point;

void ui_load();
void sub_37C8();
void update_viewport();
void sub_CF8(unsigned char *data, struct viewport_data *vp);
void draw_viewport();
void ui_draw();
void ui_draw_full();
void ui_clean();

void ui_header_reset();
void ui_header_draw();
void ui_header_set_byte(unsigned char byte);

void ui_draw_box_segment(uint8_t chr);
void ui_draw_chr_piece(uint8_t chr);
void sub_35A0(uint8_t piece_index);
void draw_pattern(struct ui_rect *rect);
void ui_set_background(uint16_t val);
void ui_draw_string(void);
void ui_draw_solid_color(uint8_t color, uint16_t line_num,
    uint16_t inset, uint16_t count);
void reset_ui_background();

void ui_rect_expand();
void ui_rect_shrink();

int ui_adjust_rect(uint8_t input);

int ui_rect_redraw(uint8_t input);
void ui_set_byte_3236(uint8_t val);
uint8_t ui_get_byte_3236();
void init_viewport_memory();
void viewport_save();
void sub_4C95(struct resource *r);
void draw_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

unsigned char *ui_get_minimap_viewport();
unsigned char *ui_get_viewport_mem();

void ui_viewport_reset();
void ui_update_viewport(size_t vp_offset);
void ui_set_viewport_width(int new_width);
void ui_set_viewport_height(int new_height);
void ui_set_viewport_offset(int new_offset);

void zero_out_2AAA();

#ifdef __cplusplus
}
#endif

#endif /* DW_UI_H */
