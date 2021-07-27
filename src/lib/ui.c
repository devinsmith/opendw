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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine.h"
#include "offsets.h"
#include "resource.h"
#include "tables.h"
#include "ui.h"
#include "utils.h"
#include "vga.h"

struct pic_data {
  uint8_t width;
  uint8_t height;
  uint8_t offset_delta;
  uint8_t y_pos; // Starting line.
  unsigned char *data;
};

uint8_t ui_drawn_yet = 0; // 0x268E

static void draw_indexed_picture9();
static void sub_1F54(uint8_t al);
static void sub_27CC();

void (*ui_draw_funcs[5])() = {
  draw_indexed_picture9,
  draw_viewport,
  reset_game_state,
  ui_header_draw,
  sub_27CC
};

static struct ui_rect data_268F;
// 0x2697
struct ui_rect draw_rect;

// 0x2794-0x27CB
static unsigned char data_2794[56] = {
  0x00, 0xB8, 0x28, 0xC0, 0x00, 0x98, 0x01, 0xB8, // 0x2794-0x279B
  0x27, 0x98, 0x28, 0xB8, 0x00, 0x90, 0x28, 0x98, // 0x279C-0x27A3
  0x27, 0x00, 0x28, 0x90, 0x1B, 0x00, 0x27, 0x20, // 0x27A4-0x27AB
  0x00, 0x00, 0x02, 0x90, 0x02, 0x00, 0x04, 0x08, // 0x27AC-0x27B3
  0x14, 0x00, 0x16, 0x08, 0x16, 0x00, 0x1B, 0x90, // 0x27B4-0x27BB
  0x02, 0x08, 0x16, 0x90, 0x1B, 0x08, 0x27, 0x78, // 0x27BC-0x27C3
  0x04, 0x00, 0x14, 0x08, 0x01, 0x98, 0x27, 0xB8  // 0x27C4-0x27CB
};

// 0x3797
static unsigned char color_data[] = {
  0x00, 0xFF, 0xCC, 0xAA, 0x99
};

static uint8_t byte_3236 = 0;
// 0x32BF
struct ui_point draw_point;

// 0x320C
struct ui_string_line ui_string = { 0 };

// 0x2AC3
uint8_t data_2AC3[0x19];

static int loaded = 0;

// At 0x4F11 in memory.
// 10880 bytes. (136 x 80)
static unsigned char *viewport_memory; // 0x4F11
static unsigned char *viewport_mem_save; // 0x4F13
static const int viewport_mem_sz = 10880;
unsigned short word_4F15; // 0x4F15
unsigned short word_4F17; // 0x4F17

// Viewport metadata.
// 0x6748
unsigned char viewport_metadata[] = {
  0x58, 0x67, 0x00, 0x00,
  0x84, 0x67, 0x98, 0x00,
  0xB0, 0x67, 0x00, 0x7B,
  0xE8, 0x67, 0x98, 0x7B
};

// Populated from data extracted from 0x6748
struct viewport_data viewports[] = {
  {
    0x00, 0x00, 0x04, 0x0A, 0x00, 0x00,
    NULL
  },
  {
    0x98, 0x00, 0x04, 0x0A, 0x00, 0x00,
    NULL
  },
  {
    0x00, 0x7B, 0x04, 0x0D, 0x00, 0x00,
    NULL
  },
  {
    0x98, 0x7B, 0x04, 0x0D, 0x00, 0x00,
    NULL
  }
};

#define COLOR_BLACK 0
#define COLOR_WHITE 0xF

#define UI_PIECE_COUNT 0x2B
#define UI_BRICK_FIRST_PICTURE 0x17
static struct pic_data ui_pieces[UI_PIECE_COUNT];

// 0x288B
// Initially "Loading..."
unsigned char ui_header_loading[] = {
  0xCC, 0xEF, 0xE1, 0xE4, 0xE9, 0xEE, 0xE7, 0xAE, 0xAE, 0xAE
};

struct ui_header {
  int len; // 0x288A
  unsigned char data[16]; // 0x288B
};

static struct ui_header ui_header;

// 0x3598
static uint8_t prev_bg_index = 0;
// 0x3599
static uint8_t curr_bg_index = 0;

// 0x359A
static uint16_t current_background = 0xFFFF;
// 0x359C
static uint16_t backgrounds[2] = { 0xFFFF, 0x0000 };

/* D88 */
static void process_quadrant(const struct viewport_data *d, unsigned char *data)
{
  int newx, sign, ax;
  int word_104A;
  uint16_t offset;

  // sar 36C0, 1
  sign = d->xpos & 0x8000;
  newx = d->xpos >> 1;
  newx |= sign;

  ax = d->runlength;
  word_104A = ax;
  ax += d->xpos;
  ax -= word_1053;
  if (ax > 0) {
#if 0
    word_104A -= ax;
    if (word_104A <= 0)
      return;
#endif
  }

  printf("0x%02X 0x%02X, 0x%02X\n", d->xpos, d->ypos, newx);
  offset = get_offset(d->ypos);
  offset += newx;
  printf("Offset: %04x (%d bytes)\n", offset, d->numruns * d->runlength);
  unsigned char *p = data + offset;
  unsigned char *q = d->data + 4;
  for (int i = 0; i < d->numruns; i++) {
    for (int j = 0; j < d->runlength; j++) {
      unsigned char val = *q;
      unsigned char dval = *p;

      dval = dval & get_and_table(val);
      dval = dval | get_or_table(val);

      //printf("(%02x %02x) ", dval, val);
      *p = dval;
      p++;
      q++;
    }
    offset += word_1055;
    p = data + offset;
    //printf("\n");
  }
}

// 0xDEB
static void sub_DEB(const struct viewport_data *d, unsigned char *data)
{
  uint16_t ax, bx, old_ax, newx, cx;
  uint16_t newy, dx;
  uint8_t al, dl;
  int sign;
  uint16_t word_104A;
  uint8_t byte_104C;
  uint16_t offset;

  // sar 36C0, 1
  sign = d->xpos & 0x8000;
  newx = d->xpos >> 1;
  newx |= sign;

  dl = 0;

  ax = d->runlength;
  word_104A = ax;
  ax += newx;
  old_ax = ax;
  ax -= word_1053;
  if (ax <= old_ax) {
    // 0x0E06
    word_104A -= ax;
    printf("%s 0xE06 unhandled\n", __func__);
    exit(1);
  }
  // 0xE0F
  byte_104C = dl;
  newy = d->ypos;
  newy = newy << 1;

  offset = get_offset(d->ypos);
  offset += newx;

  ax = ax & 0xFF;

  // 0xE27
  cx = word_104A;
  // 0xE35
  unsigned char *p = data + offset;
  unsigned char *ds = d->data + 4;

  // 1048 = 13 ?
  for (int i = 0; i < d->numruns; i++) {
    for (int j = 0; j < cx; j++) {
      al = *ds;
      ax = (ax & 0xFF00) | al;
      bx = ax;

      dx = p[0];
      dx += p[1] << 8;

      dx &= get_and_table_B452(bx);
      dx |= get_or_table_B652(bx);

      *p = dx & 0xFF;
      p++;
      *p = (dx & 0xFF00) >> 8;
      printf("(0x%04X) ", dx);
      ds++;
    }
    // 0xE4C
    *p = (dx & 0xFF);
    if (byte_104C < 0x80) {
      p++;
      *p = (dx & 0xFF00) >> 8;
    }
    // 0x3A + 0x13
    // offset += 1055
    offset += word_1055;
    p = data + offset;
    printf("\n");
  }
}

// 0xEC5
static void sub_EC5(const struct viewport_data *d, unsigned char *data)
{
  uint16_t ax;
  int bx;
  int sign, word_104A;
  uint16_t offset, save;
  uint16_t dx;
  unsigned char *ds = d->data + 4;
  unsigned char *base;
  uint8_t al;

  ax = d->xpos;
  ax = -ax;
  // sar ax, 1
  sign = ax & 0x8000;
  ax = ax >> 1;
  ax |= sign;

  bx = d->runlength;
  bx -= ax;
  word_104A = bx;
  if (word_104A <= 0) {
    return;
  }

  // 0xEDB
  ax = ax & 0xFF;
  ds += ax; // add si, ax
  bx = d->ypos;
  offset = get_offset(d->ypos);
  offset--;

  // 0xEEE
  for (int i = 0; i < d->numruns; i++) {
    save = offset;
    base = ds;
    unsigned char *p = data + offset;
    al = *ds++;
    bx = al;
    dx = p[0];
    dx += p[1] << 8;
    dx &= get_and_table_B452(bx);
    dx |= get_or_table_B652(bx);
    p++;
    *p = (dx & 0xFF00) >> 8;

    // 0xF10
    for (int j = 0; j < (word_104A - 1); j++) {
      al = *ds++;
      bx = al;
      dx = p[0];
      dx += p[1] << 8;

      dx &= get_and_table_B452(bx);
      dx |= get_or_table_B652(bx);

      *p = dx & 0xFF;
      p++;
      *p = (dx & 0xFF00) >> 8;
    }
    offset = save;
    offset += word_1055;
    p = data + offset;
    base += d->runlength;
    ds = base;
  }
}

// 0xE6D
static void sub_E6D(const struct viewport_data *d, unsigned char *data)
{
  uint16_t ax, cx, dx;
  uint8_t al;
  uint16_t offset;
  uint16_t word_104A;
  int bx, sign;
  int si = 4;


  ax = d->xpos;
  ax = -ax;
  al = ax & 0xFF;

  // sar al, 1
  sign = al & 0x80;
  al = al >> 1;
  al |= sign;
  ax = (ax & 0xFF00) | al;

  bx = d->runlength;
  bx -= ax;

  word_104A = bx;
  if (bx <= 0)
    return;

  // 0xE83
  ax = ax & 0xFF;

  si += ax;
  bx = d->ypos;
  dx = get_offset(bx);
  ax = ax & 0xFF;

  // 0xE95
  for (int i = 0; i < d->numruns; i++) {
    cx = word_104A;
    offset = dx;
    // bp = si
    unsigned char *p = data + offset;
    unsigned char *ds = d->data + si;
    printf("%s: lodsb: 0x%02X\n", __func__, *ds);

    // 0xE9E
    for (int j = 0; j < cx; j++) {
      al = *ds++;
      bx = al;
      al = *p;
      al &= get_and_table(bx);
      al |= get_or_table(bx);
      *p = al;
      p++;
    }
    si += d->runlength;
    offset += word_1055;
  }
}

// 0x1060
void draw_viewport()
{
  int rows = 0x88;
  int cols = 0x50;
  int line_num = 8; // Can be increased on 10D2

  // 2752 (0x0a)
  //
  sub_1F54(0x0a);

  uint8_t *framebuffer = vga->memory();

  // 0x88 x 0x50
  /* see 0x1060 */
  const unsigned char *src = viewport_memory;
  for (int y = 0; y < rows; y++) {
    uint16_t fb_off = get_line_offset(line_num) + 0x10;
    for (int x = 0; x < cols; x++) {
      uint8_t al = *src++;
      int hi, lo;

      /* Each nibble represents a color */
      /* for example 0x82 represents color 8 then color 2. */
      hi = (al >> 4) & 0xf;
      lo = al & 0xf;

      framebuffer[fb_off++] = hi;
      framebuffer[fb_off++] = lo;
    }
    line_num++;
  }
  vga->update();
}

/* 0x35A0 -> 0x3679 */
void draw_ui_piece(const struct pic_data *pic)
{
  uint16_t starting_off = get_line_offset(pic->y_pos);
  starting_off += (pic->offset_delta * 4);
  uint16_t fb_off = starting_off;
  printf("Line number: %d - FB offset: 0x%04x\n", pic->y_pos, fb_off);
  unsigned char *src = pic->data;
  uint8_t *framebuffer = vga->memory();

  for (int y = 0; y < pic->height; y++) {
    for (int x = 0; x < pic->width; x++) {
      uint8_t al = *src++;
      int hi, lo;

      /* Each nibble represents a color */
      /* for example 0x82 represents color 8 then color 2. */
      hi = (al >> 4) & 0xf;
      lo = al & 0xf;

      framebuffer[fb_off++] = hi;
      framebuffer[fb_off++] = lo;
    }
    starting_off += 0x140;
    fb_off = starting_off;
  }
  vga->update();
}

/* 0x36C8 */
void ui_draw_solid_color(uint8_t color_idx, uint16_t inset,
    uint16_t count, uint16_t line_num)
{
  // early exit.
  if (count <= inset) {
    return;
  }
  count -= inset;
  count = count << 1;

  // bx = color
  uint8_t color = color_data[color_idx];
  color = color & 0x0F;

  uint16_t fb_off = get_line_offset(line_num);
  inset = inset << 2;
  fb_off += inset;
  uint8_t *framebuffer = vga->memory();

  for (uint16_t i = 0; i < count; i++) {
    framebuffer[fb_off++] = color;
    framebuffer[fb_off++] = color;
  }
}

// 0x3351 (sort of).
// x stored in DX, y = DI
static void draw_character(int x, int y, const unsigned char *chdata)
{
  uint8_t color = COLOR_WHITE; // bh.
  uint8_t ah = (current_background >> 8) & 0xFF;

  uint8_t *framebuffer = vga->memory();
  uint16_t fb_off = get_line_offset(y);
  fb_off += (x << 3); // 8 bytes

  for (int j = 0; j < 8; j++) {
    uint8_t bl;
    uint8_t al = *chdata++;
    al = al ^ ah;
    bl = al;
    for (int i = 0; i < 8; i++) {
      color = COLOR_BLACK;
      bl = (al << 1) & 0xFF;
      if (bl < al) {
        color = COLOR_WHITE;
      }
      framebuffer[fb_off++] = color;
      al = bl;
    }
    fb_off += 0x138;
  }
}

// 0x2AE3
static void zero_out_2AAA()
{
  memset(data_2AAA, 0, sizeof(data_2AAA));
}

static void sub_27CC()
{
  draw_rect.x = 1;
  draw_rect.y = 0x98;
  draw_rect.w = 0x27;
  draw_rect.h = 0xB8;
  draw_pattern(&draw_rect);
}

// 0x3380
void draw_pattern(struct ui_rect *rect)
{
  int num_lines = rect->h - rect->y;
  int dx = rect->w - rect->x;
  int starting_line = rect->y;
  zero_out_2AAA();
//  printf("Number of lines: %d\n", num_lines);
//  printf("Line: %d\n", starting_line);
//  printf("DX: 0x%04x\n", dx);

  uint16_t ax = 0xFFFF; // word_359A (color)

  uint8_t *framebuffer = vga->memory();

  // 0x3417
  int x_pos = rect->x << 3;
  dx = dx << 2;
  ax = ax & 0x0F0F;
  printf("x_pos: %d\n", x_pos);
  printf("DX: 0x%04x\n", dx);

  for (int i = 0; i < num_lines; i++) {
    uint16_t fb_off = get_line_offset(starting_line);
    fb_off += x_pos;
    for (int j = 0; j < dx; j++) {
      int color1 = (ax >> 8) & 0xFF;
      int color2 = (ax & 0x00FF);
      framebuffer[fb_off++] = color1;
      framebuffer[fb_off++] = color2;
    }
    starting_line++;
  }

  draw_point.x = draw_rect.x;
  byte_3236 = draw_rect.x;
  draw_point.y = draw_rect.y;
  ui_string.len = 0;
}

// 0x26B8
void ui_draw_full(void)
{
  ui_draw_string();
  if (ui_drawn_yet != 0) {
    ui_draw();
  }
  ui_drawn_yet = 0;
  draw_rect.x = 1;
  draw_rect.y = 0x98;
  draw_rect.w = 0x27;
  draw_rect.h = 0xB8;
  draw_point.x = 1;
  draw_point.y = 0x98;
}

// 0x4C22
void draw_indexed_picture9()
{
  draw_ui_piece(&ui_pieces[9]);

  word_4C31[0] = 0;
  word_4C31[1] = 0;
  word_4C31[2] = 0;
  word_4C31[3] = 0;
}

static void sub_1F54(uint8_t al)
{
  if (byte_2476 == 0) {
    return;
  }
  // 0x1F5B
  printf("%s: 0x1F5B unhandled.\n", __func__);
  exit(1);
}

// 0x35A0
void sub_35A0(uint8_t piece_index)
{
  if (piece_index >= UI_PIECE_COUNT) {
    printf("%s: Piece count is too high! 0x%02X\n", __func__, piece_index);
    exit(1);
  }

  sub_1F54(9);

  draw_ui_piece(&ui_pieces[piece_index]);
}

/* 0x26E9 */
void ui_draw()
{
  ui_drawn_yet = 0;

  for (int counter = 0; counter < 14; counter++) {
    if (ui_adjust_rect(counter) == 1) {
      if (counter >= 9) {
        int fidx = counter - 9;
        ui_draw_funcs[fidx]();
      } else {
        draw_ui_piece(&ui_pieces[counter]);
      }
    }
  }
  vga->update();
}

// 0x2824
void ui_header_draw()
{
  ui_set_background(0x10);

  // Calculate label header starting position.
  int header_start = sizeof(ui_header.data) - ui_header.len;
  header_start = header_start >> 1;
  header_start += 4;

  for (int i = 4; i < header_start; i++) {
    draw_ui_piece(&ui_pieces[i + UI_BRICK_FIRST_PICTURE]);
  }

  for (int i = 0; i < ui_header.len; i++) {
    draw_character(i + header_start, 0, get_chr(ui_header.data[i]));
  }

  for (int i = ui_header.len + header_start; i < 0x14; i++) {
    draw_ui_piece(&ui_pieces[i + UI_BRICK_FIRST_PICTURE]);
  }
  reset_ui_background();
}

void ui_load()
{
  /* Viewport data is stored in the dragon.com file */
  viewports[0].data = com_extract(0x6758, 4 + (4 * 0xA));
  viewports[1].data = com_extract(0x6784, 4 + (4 * 0xA));
  viewports[2].data = com_extract(0x67B0, 4 + (4 * 0xD));
  viewports[3].data = com_extract(0x67E8, 4 + (4 * 0xD));

  unsigned char *ui_piece_offsets_base = com_extract(0x6AE0, UI_PIECE_COUNT * 2);
  unsigned char *ui_piece_offsets = ui_piece_offsets_base;
  printf("UI Pieces:\n");
  dump_hex(ui_piece_offsets, UI_PIECE_COUNT * 2);
  for (size_t ui_idx = 0; ui_idx < UI_PIECE_COUNT; ui_idx++) {
    uint16_t ui_off = *ui_piece_offsets++;
    ui_off += *ui_piece_offsets++ << 8;
    printf("Piece: %zu Offset: %04x\n", ui_idx, ui_off);
    /* Next 4 bytes are encoded into pic data */
    unsigned char *piece_struct = com_extract(ui_off, 4);
    unsigned char *org = piece_struct;
    ui_pieces[ui_idx].width = *piece_struct++;
    ui_pieces[ui_idx].height = *piece_struct++;
    ui_pieces[ui_idx].offset_delta = *piece_struct++;
    ui_pieces[ui_idx].y_pos = *piece_struct;
    free(org);
    size_t data_sz = ui_pieces[ui_idx].width * ui_pieces[ui_idx].height;
    ui_pieces[ui_idx].data = com_extract(ui_off + 4, data_sz);
  }
  free(ui_piece_offsets_base);

  memcpy(ui_header.data, ui_header_loading, strlen("Loading..."));
  ui_header.len = strlen("Loading...");
  loaded = 1;
}

void ui_clean()
{
  free(viewports[0].data);
  free(viewports[1].data);
  free(viewports[2].data);
  free(viewports[3].data);

  for (size_t ui_idx = 0; ui_idx < UI_PIECE_COUNT; ui_idx++) {
    free(ui_pieces[ui_idx].data);
  }

  free(viewport_memory);
}

void ui_header_reset()
{
  ui_header.len = 0;
}

// 0x27FA (not really, but close enough).
void ui_header_set_byte(unsigned char byte)
{
  ui_header.data[ui_header.len++] = byte;
}

// 0x3237
void ui_draw_chr_piece(uint8_t chr)
{
  if ((chr & 0x80) == 0) {
    int16_t bx = (int16_t)draw_point.y;
    bx -= draw_rect.y;
    if (bx > 0) {
      bx = bx >> 3;

      data_2AC3[bx] = chr;
      data_2AAA[bx] = 0xFF;
    }
  }
  if (chr == 0x8D) {
    draw_point.x = draw_rect.x;
    uint8_t al = draw_point.y;
    al += 8;
    if (al > draw_rect.h) {
      printf("BP CS:3275\n");
      exit(1);
    }
    draw_point.y = al;
    return;
  }
  draw_character(draw_point.x, draw_point.y, get_chr(chr));
  draw_point.x++;
}

// 0x269F
void ui_draw_box_segment(uint8_t chr)
{
  // Draw corner box.
  ui_draw_chr_piece(chr);
  chr++;

  while (draw_point.x < draw_rect.w - 1) {
    ui_draw_chr_piece(chr);
  }

  chr++;
  ui_draw_chr_piece(chr);

  vga->update();
}

// 0x3578
void ui_set_background(uint16_t val)
{
  val = val >> 3;
  val = val & 2;

  if (val == 2) {
    current_background = backgrounds[1];
  } else {
    current_background = backgrounds[0];
  }
  prev_bg_index = curr_bg_index;
  curr_bg_index = (val & 0xFF);
}

// 0x3575
void reset_ui_background()
{
  ui_set_background(prev_bg_index);
}

void ui_set_byte_3236(uint8_t val)
{
  byte_3236 = val;
}

uint8_t ui_get_byte_3236()
{
  return byte_3236;
}

// 0x3177
void ui_draw_string(void)
{
  uint16_t i;
  for (i = 0; i < ui_string.len; i++) {
    uint8_t al = ui_string.bytes[i];
    ui_draw_chr_piece(al);
  }
  ui_string.len = 0;

  // 0x318A
  byte_3236 = draw_point.x;
  vga->update();
}

// 0x2720
void ui_rect_expand()
{
  draw_rect.h += 8;
  draw_rect.y -= 8;
  draw_rect.x--;
  draw_rect.w++;
}

// 0x2739
void ui_rect_shrink()
{
  draw_rect.y += 8;
  draw_rect.h -= 8;
  draw_rect.x++;
  draw_rect.w--;
}

// Uses carry flag as a boolean
// 0x2752
int sub_2752(uint8_t input)
{
  if (ui_drawn_yet == 0)
    return 0;

  return ui_adjust_rect(input);
}

// 0x2759
int ui_adjust_rect(uint8_t input)
{
  uint16_t ax, si;
  uint8_t al;

  ax = input;
  ax = ax << 2;
  si = ax;

  ui_rect_expand();

  // 0x2764
  al = data_2794[si];
  if (al > draw_rect.w) {
    ui_rect_shrink();
    return 0;
  }
  al = data_2794[si + 1];
  if (al > draw_rect.h) {
    ui_rect_shrink();
    return 0;
  }
  al = data_2794[si + 2];
  if (al < draw_rect.x) {
    ui_rect_shrink();
    return 0;
  }
  al = data_2794[si + 3];
  if (al < draw_rect.y) {
    ui_rect_shrink();
    return 0;
  }
  ui_rect_shrink();
  return 1;
}

// 0xCF8
// extract and process viewport data.
void sub_CF8(unsigned char *data, struct viewport_data *vp)
{
  uint8_t al;
  uint16_t ax, bx;
  unsigned char *ds = data;

  vp->runlength = *ds++;
  vp->numruns = *ds++;

  al = *ds++;
  ax = (int8_t)al;
  if (byte_104E >= 0x80) {
    // neg ax;
    ax = -ax;
  }
  vp->xpos += ax;
  if (vp->runlength >= 0x80 && byte_104E >= 0x80) {
    vp->xpos--;
  }
  vp->runlength &= 0x7F;
  al = *ds++;
  ax = (int8_t)al;

  if (byte_104E >= 0x40) {
    ax = -ax;
  }
  vp->ypos += ax;

  bx = vp->xpos;
  bx &= 1;
  bx = bx << 1;
  if (vp->xpos >= 0x8000) {
    bx |= 4;
  }

  // 0xD52
  if (byte_104E >= 0x80) {
    bx |= 8;
  }

  // 0xD5C
  //
  ax = word_1053;
  if (byte_104E >= 0x40) {
    ax = -ax;
  }
  // 0xD67
  word_1055 = ax;

  // 0xD78 offset
  switch (bx) {
  case 0:
    process_quadrant(vp, viewport_memory);
    break;
  case 2:
    sub_DEB(vp, viewport_memory);
    break;
  case 4:
    sub_E6D(vp, viewport_memory);
    break;
  case 6:
    sub_EC5(vp, viewport_memory);
    break;
  default:
    printf("%s: An unhandled BX (0x%04X) was specified.\n", __func__, bx);
    exit(1);
    break;
  }
}

// 0x0CA0
void update_viewport()
{
  uint16_t di, cx;
  uint16_t i;

  // 0x4F11
  unsigned char *ds = viewport_memory;
  di = 0;
  cx = 0x88;

  // 0xCAD
  for (i = 0; i < cx; i++) {
    ds[di] &= 0x0F;
    ds[di + 0x4F] &= 0xF0;
    di += 0x50;
  }
  printf("di = 0x%04X\n", di);
  byte_104E = 0;

  // 0xCBF
  int vidx = 3;
  while (vidx >= 0) {
    int offset_idx = vidx << 2;
    uint16_t offset;

    // Load viewport data.
    struct viewport_data *p = &viewports[vidx];
    offset = viewport_metadata[offset_idx];
    offset += viewport_metadata[offset_idx + 1] << 8;

    p->xpos = viewport_metadata[offset_idx + 2];
    p->ypos = viewport_metadata[offset_idx + 3];

    // Data already loaded in ui_load.
    //
    sub_CF8(p->data, p);

    vidx--;
  }
  draw_viewport();
}

// 0x25E0
void draw_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  int identical = 1;
  int needs_resize = 0;

  data_268F.x = x;
  data_268F.y = y;
  data_268F.w = w;
  data_268F.h = h;

  ui_draw_string();
  if (ui_drawn_yet != 0) {
    ui_rect_expand();
    if (draw_rect.x < data_268F.x ||
        draw_rect.y > data_268F.y ||
        draw_rect.w < data_268F.w ||
        draw_rect.h > data_268F.h) {
      needs_resize = 1;
      identical = 1;
      // 0x2632
    } else {
      // 2623
      identical = memcmp(&data_268F, &draw_rect, sizeof(data_268F));
    }
  }
  // 0x2632
  if (needs_resize) {
    ui_rect_shrink();
    ui_draw();
  }

  if (identical != 0) {
    // 0x2638
    memcpy(&draw_rect, &data_268F, sizeof(data_268F));
    draw_point.x = draw_rect.x;
    draw_point.y = draw_rect.y;

    // 0x32BF, 0x32C1, 0x80
    printf("sub_269F(%d, %d, 0x80)\n", draw_point.x, draw_point.y);
    // 0x269F
    ui_draw_box_segment(0x80);

    // loc_2668
    // Draw left and right sides.
    while (draw_point.y < draw_rect.h - 8) {
      draw_point.x = draw_rect.x;
      draw_point.y += 8;
      ui_draw_chr_piece(0x83);
      draw_point.x = draw_rect.w - 1;
      ui_draw_chr_piece(0x84);
    }
    draw_point.x = draw_rect.x;
    ui_draw_box_segment(0x85);
  }

  // 0x2683
  ui_drawn_yet = 0xFF;
  ui_rect_shrink();
  draw_pattern(&draw_rect);
  vga->update();
}

// 0x37C8
void sub_37C8()
{
  sub_4D82();

  memset(viewport_memory, 0, viewport_mem_sz);
  byte_4F0F= 0xFF;

  update_viewport();
}

// 0x4CFA
void viewport_save()
{
  // repe movsw
  memcpy(viewport_mem_save, viewport_memory, viewport_mem_sz);
}

static void sub_4CB2(unsigned char *es, struct resource *r)
{
  unsigned char *ds = r->bytes;

  // Offsets, x, y, num rows, num cols ?
  unsigned short xpos = *ds++; // dx
  unsigned short ypos = *ds++; // di
  unsigned short runlen = *ds++; // ax
  runlen -= xpos;
  xpos += 5;

  unsigned short numruns = *ds++; // bp
  unsigned short bx = 0;

  // 4CDC
  for (int i = ypos; i < numruns; i++) {
    uint16_t offset = get_offset(i);
    offset += xpos;

    unsigned char *p = es + offset;
    for (int j = 0; j < runlen; j++) {
      // 4CDC
      unsigned char data = *ds;
      unsigned char dval = *p;

      bx = bx ^ data;
      dval = dval & get_and_table(bx);
      dval = dval | get_or_table(bx);

      *p = dval;
      p++;
      ds++;
    }
  }
}

// 0x4D10
void viewport_restore()
{
  memcpy(viewport_memory, viewport_mem_save, viewport_mem_sz);
}

// 0x4D26
void sub_4D26()
{
  memset(viewport_mem_save, 0x66, viewport_mem_sz);
}

// 0x4C95
// Draw random encounter (graphic in resource r)
void sub_4C95(struct resource *r)
{
  // Segment and offset (will be later loaded into ds:si)
  // of resource r.
  // word_4F15 = bx; // 0 ? (offset)
  // word_4F17 = cx; // 0x2979 (Segment)

  viewport_save();

  // 01DD:4CA0  A1114F  mov  ax,[4F11]  ds:[4F11]=0FC4
  sub_4CB2(viewport_memory, r);
  draw_viewport();
  viewport_restore();

  sub_4D26();

  sub_4CB2(viewport_mem_save, r);
}

void init_viewport_memory()
{
  viewport_memory = malloc(viewport_mem_sz);
  viewport_mem_save = malloc(viewport_mem_sz);
}
