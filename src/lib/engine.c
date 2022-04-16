/*
 * Copyright (c) 2020-2022 Devin Smith <devin@devinsmith.net>
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "compress.h"
#include "engine.h"
#include "offsets.h"
#include "player.h"
#include "resource.h"
#include "state.h"
#include "tables.h"
#include "ui.h"
#include "utils.h"
#include "vga.h"

/* Represents the engine that Dragon wars runs.
 *
 * It appears that Dragon Wars executes a script where each op code
 * does some action.
 *
 * Lots of global variables here until we can figure out how they are used. */

uint16_t counter_104D;

unsigned char byte_104E;
// 104F is technically a dword with segment:offset.
uint16_t word_104F = 0; // offset into 1051
struct resource *word_1051;

uint16_t word_11C0 = 0;
uint16_t word_11C2 = 0;
uint16_t word_11C4 = 0;
uint16_t word_11C6 = 0;
uint16_t word_11C8 = 0;

uint16_t word_11CA = 0;
uint16_t word_11CC = 0;

uint8_t byte_1949 = 0;

// Minimap offsets?
uint8_t byte_1960 = 0;
uint8_t byte_1961 = 0;
uint8_t byte_1962 = 0;
uint8_t byte_1964 = 0;
uint8_t byte_1966 = 0;

// Viewport offsets
unsigned short data_1997[] = { 0x0000, 0x0010, 0x0028, 0x0040, 0x0058, 0x0070, 0x0088, 0x00A0 };
// Viewport heights
unsigned short data_19A7[] = { 0x0010, 0x0028, 0x0040, 0x0058, 0x0070, 0x0088, 0x00A0, 0x00A8 };
unsigned short data_19B7[] = { 0x0030, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020 };

// 0x19FE - 0x1A0F (each triplet means something)
unsigned short data_19FE[] = { 0x0000, 0x0018, 0x0000,
  0xFFF8, 0x0010, 0x0000,
  0xFFF8, 0x0010, 0x0002};

uint8_t byte_1CE1 = 0;
uint8_t byte_1CE2 = 0;

uint8_t byte_1BE5 = 0;

// 0x1C63
// Typically will be (player number * 0x200) + 0xC960
uint16_t player_base_offset = 0;

uint8_t byte_1E1F = 0;
uint8_t byte_1E20 = 0;

// This is an unknown size, currently guessing at
// 0x1E21 - 0x1F0F
unsigned char *data_1E21;

// 0x1EB9, 2 bytes, since there's a function at 0x1EBB
unsigned char data_1EB9[] = { 0xC2, 0x00 };

uint8_t byte_1F07 = 0;
uint8_t byte_1F08 = 0;

// 0x246D
uint16_t word_246D;

unsigned char byte_2476;

uint16_t word_2AA2;
unsigned char *word_2AA4;

uint8_t byte_2AA6;
// 0x2AA7
uint16_t word_2AA7;
uint8_t byte_2AA9;
uint8_t data_2AAA[32] = { 0 };

// 0x2D09
uint16_t random_seed = 0x1234;

uint16_t word_2DD7 = 0xFFFF;
uint16_t word_2DD9 = 0xFFFF;
uint8_t data_2DDB[160] = { 0 };

uint16_t word_36C0;
uint16_t word_36C2;
uint16_t g_linenum; // 36C4

uint8_t byte_3867 = 0;
uint8_t byte_387F = 0;

uint8_t byte_3AE1 = 0;
uint16_t word_3AE2 = 0;
uint16_t word_3AE4 = 0;
uint16_t word_3AE6 = 0;
uint16_t word_3AE8 = 0;
uint16_t word_3AEA = 0;
// 0x3AEC
uint16_t saved_stack = 0;

uint16_t word_3ADB = 0;

// "Bit extraction"
// 0x1CEF
struct bit_extractor bit_extractor_info = { 0 };

/* 0x3ADD */
const struct resource *running_script = NULL;
const struct resource *word_3ADF = NULL;

uint16_t word_42D6 = 0;
uint16_t word_4454 = 0;

// 0x4A5B
unsigned short data_4A5B[] = { 0x005A };

// 0x4A99
unsigned char data_4A99[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

// 0x49AB (compressed Yes/No bytes)
unsigned char data_49AB[] = { 0xFE, 0x6, 0x97, 0x7F, 0x2B, 0xC0 };

unsigned char byte_4F0F;
unsigned char byte_4F10 = 0;

// 0x4F19 - 0x4F28 ? (how big is this really?)
unsigned char data_4F19[17] = { 0 };
const struct resource *word_4F29 = NULL;
uint8_t byte_4F2B = 0;

// Another function pointer.
void (*word_5038)(unsigned char *dest, unsigned int offset);

// 0x5303
// Field of vision offsets?
unsigned short data_5303[] = {
  0x0016, 0x002E, 0x0046, 0x005E
};

// 0x530B - 0x536A
// Field of vision offsets?
static unsigned char data_530B[] = {
  0xff, 0x03, 0x00, 0x03, 0x01, 0x03, 0xff, 0x02, 0x00, 0x02, 0x01, 0x02,
  0xff, 0x01, 0x00, 0x01, 0x01, 0x01, 0xff, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x03, 0x01, 0x03, 0x00, 0x03, 0xff, 0x02, 0x01, 0x02, 0x00, 0x02, 0xff,
  0x01, 0x01, 0x01, 0x00, 0x01, 0xff, 0x00, 0x01, 0x00, 0x00, 0x00, 0xff,
  0x01, 0xfd, 0x00, 0xfd, 0xff, 0xfd, 0x01, 0xfe, 0x00, 0xfe, 0xff, 0xfe,
  0x01, 0xff, 0x00, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0xff, 0x00,
  0xfd, 0xff, 0xfd, 0x00, 0xfd, 0x01, 0xfe, 0xff, 0xfe, 0x00, 0xfe, 0x01,
  0xff, 0xff, 0xff, 0x00, 0xff, 0x01, 0x00, 0xff, 0x00, 0x00, 0x00, 0x01
};

unsigned char *data_5521;
uint8_t byte_551E;
uint16_t word_551F;

// 0x558F - 0x55BE
unsigned short data_558F[] = {
  0x0020, 0x0000, 0x0080, 0xFFC0,
  0x0080, 0x0020, 0xFFC0, 0x0080,
  0x0030, 0x0020, 0x0070, 0xFFF0,
  0x0070, 0x0030, 0xFFF0, 0x0070,
  0x0040, 0x0030, 0x0060, 0x0020,
  0x0060, 0x0040, 0x0020, 0x0060
};

// 0x55BF - 0x55EE
unsigned short data_55BF[] = {
  0x0010, 0x0000, 0x0000, 0x0010,
  0x0010, 0x0010, 0x0010, 0x0010,
  0x0020, 0x0010, 0x0010, 0x0020,
  0x0020, 0x0020, 0x0020, 0x0020,
  0x0030, 0x0020, 0x0020, 0x0030,
  0x0030, 0x0030, 0x0030, 0x0030
};

// 0x55EF - 0x561E
unsigned short data_55EF[] = {
  0x0016, 0x000A, 0x000B, 0x0015,
  0x0017, 0x008A, 0x0089, 0x008B,
  0x0013, 0x0007, 0x0008, 0x0012,
  0x0014, 0x0087, 0x0086, 0x0088,
  0x0010, 0x0004, 0x0005, 0x000F,
  0x0011, 0x0084, 0x0083, 0x0085
};

// 0x561F - 0x564E
unsigned short data_561F[] = {
  0x0004, 0x000C, 0x000C, 0x0004,
  0x0004, 0x0004, 0x0004, 0x0004,
  0x0006, 0x000E, 0x000E, 0x0006,
  0x0006, 0x0006, 0x0006, 0x0006,
  0x0008, 0x0010, 0x0010, 0x0008,
  0x0008, 0x0008, 0x0008, 0x0008
};

// 0x564F - 0x567E
unsigned short data_564F[] = {
  0x0001, 0x0000, 0x0080, 0x0001,
  0x0001, 0x0000, 0x0000, 0x0000,
  0x0001, 0x0000, 0x0080, 0x0001,
  0x0001, 0x0000, 0x0000, 0x0000,
  0x0001, 0x0000, 0x0080, 0x0001,
  0x0001, 0x0000, 0x0000, 0x0000
};

// 0x567F - 0x5690 (x) and
// 0x5691 - 0x56A2 (y)
struct ui_point ground_points[] = {
  { 16, 120 }, { 0, 120 }, { 128, 120 }, // Bottom
  { 32, 104 }, { 0, 104 }, { 112, 104}, // Middle
  { 48, 88 }, { 0, 88 }, { 96, 88 } // Top
};

// Used as viewport data.
// 0x56A3 - 0x56B4
unsigned short data_56A3[] = {
  0x000A, 0x0009, 0x000B,
  0x0007, 0x0006, 0x0008,
  0x0004, 0x0003, 0x0005
};

// 0x56B5 - 0x56C6
// sprite image offsets within image data.
unsigned short sprite_indices[] = {
  18, 16, 20, 12, 10, 14, 6, 4, 8
};

// Unknown how large this is.
// But also referenced as: 56E5 (not sure if this is correct at this point)
unsigned char data_56C6[128]; // should be 16 (not 128)
unsigned char data_56E5[128];


static struct resource *data_59E4[128];

uint16_t word_5864; // offset
unsigned char *data_5866; // data

// Unknown how large this is
// 0x5897
unsigned char data_5897[256];

// 0x4C31 - 0x4C34
unsigned char word_4C31[4];

/* Timers? */
struct timer_ctx {
  uint8_t  timer0; // 0x4C35
  uint8_t  timer1; // 0x4C36
  uint8_t  timer2; // 0x4C37
  uint16_t timer3; // 0x4C38
  uint16_t timer4; // 0x4C3A
  uint8_t  timer5; // 0x4C3C (appears to be more of a flag)
};

struct timer_ctx timers;

unsigned short data_5A04[128]; // offsets
unsigned char data_5A56[128]; // 0x5A56-0x5AD5 (? pre-loaded)

// 0x2A68
unsigned char *escape_string_table = NULL;
unsigned char *data_D760 = NULL;

// XXX:How big should these be???
// It looks like they can be 0x0E00 bytes, but we round up to 4096.
unsigned char data_CA4C[4096] = { 0 };

// The function signature for this function pointer is not entirely correct
// but we'll figure it out as we decode more of DW.
// 0x3163
void (*string_byte_handler_func)(unsigned char byte);

// Small stack, hopefully we don't use much of it.
#define STACK_SIZE 32

// virtual CPU
struct virtual_cpu {
  // registers
  uint16_t ax;
  uint16_t bx;
  uint16_t cx;
  uint16_t dx;

  uint16_t di;
  uint16_t si;

  // stack
  uint8_t stack[STACK_SIZE]; // stacks;
  uint8_t sp;

  // flags
  uint8_t cf;
  uint8_t zf;
  uint8_t sf;

  // program counter
  unsigned char *pc;
  unsigned char *base_pc;
};

/* | Bit # |  Mask  | Abbreviation       | Description                     |
 * +-------+--------+--------------------+---------------------------------+
 * | 0     | 0x0001 | CF                 | Carry flag                      |
 * | 1     | 0x0002 | Reserved, always 1 |                                 |
 * | 2     | 0x0004 | PF                 | Parity flag                     |
 * | 3     | 0x0008 | Reserved           |                                 |
 * | 4     | 0x0010 | AF                 | Adjust flag                     |
 * | 5     | 0x0020 | Reserved[3]        |                                 |
 * | 6     | 0x0040 | ZF                 | Zero flag                       |
 * | 7     | 0x0080 | SF                 | Sign flag                       |
 * | 8     | 0x0100 | TF                 | Trap flag                       |
 * | 9     | 0x0200 | IF                 | Interrupt enable flag           |
 * | 10    | 0x0400 | DF                 | Direction flag                  |
 * | 11    | 0x0800 | OF                 | Overflow flag                   |
 * | 12-13 | 0x3000 | IOPL               | I/O privilege level (286+ only) |
 * | 14    | 0x4000 | NT                 | Nested task flag (286+ only)    |
 * | 15    | 0x8000 | Reserved           |                                 |
*/
#define CARRY_FLAG_MASK 0x1
#define ZERO_FLAG_MASK 0x40
#define SIGN_FLAG_MASK 0x80

struct virtual_cpu cpu;

// 0x3854 - 0x3859
struct mouse_status mouse;

static void run_script(uint8_t script_index, uint16_t src_offset);
static void sub_CE7(struct viewport_data *vp, uint16_t sprite_offset);
static void sub_11A0(int set_11C4);
static void sub_11CE();
static void handle_byte_callback(unsigned char byte);
static void set_sb_handler_append_string();
static void append_string(unsigned char byte);
static void sub_176A();
static void sub_19C7(uint8_t val);
static void sub_1A10();
static void sub_1A72();
static void sub_1ABD(uint8_t val);
static void sub_280E();
static void sub_1BF8(uint8_t color, uint8_t y_adjust);
static void sub_28B0(unsigned char **src_ptr, unsigned char *base);
static void set_ui_header(unsigned char *base_ptr, uint16_t offset);
static void sub_2CF5();
static void sub_3F2F();
static void set_sb_handler_ui_draw_chr();
static void sub_4A79(uint8_t al);
static void sub_4D37(int al, int index, const struct resource *r);
static void sub_4D97(uint16_t index);
static void set_sign_flag();
static void clear_sign_flag();
static void sub_54D8(int x, int y);
static void sub_536B(int x, int y);
static void sub_587E();
static void cache_resources();
static void mini_map_escape();
static void mini_map_left();
static void mini_map_right();
static void mini_map_up();
static void mini_map_down();

#define NUM_FUNCS 4
static void sub_50B2();
static void sub_5088();
static void sub_5080();
static void sub_5090();

// 0x5060
void (*func_5060[NUM_FUNCS])() = {
  sub_50B2,
  sub_5088,
  sub_5080,
  sub_5090
};

// Decoded opcode calls, forward definition.
static void set_word_mode();
static void set_byte_mode();
static void op_03();
static void op_04();
static void op_05();
static void op_06();
static void op_07();
static void op_08();
static void set_word3AE2_arg();
static void load_word3AE2_gamestate();
static void op_0B();
static void op_0C();
static void op_0D();
static void op_0E();
static void op_extract_resource_data(); // op_0F
static void op_10();
static void op_11();
static void op_12();
static void set_gamestate_offset();
static void op_14();
static void op_15();
static void op_16();
static void store_data_into_resource();
static void op_18();
static void op_19();
static void op_1A();
static void op_1C();
static void op_1D();
static void op_21();
static void op_22();
static void op_23();
static void op_24();
static void inc_byte_word_3AE4();
static void op_26();
static void op_27();
static void op_28();
static void op_2A();
static void op_2B();
static void op_right_shift();
static void op_2F();
static void op_30();
static void op_31();
static void op_32();
static void op_34();
static void op_35();
static void op_38();
static void op_39();
static void op_3A();
static void op_3C();
static void op_3D();
static void op_3E();
static void op_3F();
static void op_40();
static void op_41();
static void op_42();
static void op_43();
static void op_jnz();
static void op_jz();
static void op_js();
static void op_47();
static void op_48();
static void loop(); // 49
static void op_4A();
static void op_stc();
static void op_4C();
static void op_4D();
static void op_4F();
static void op_51();
static void op_52();
static void op_53();
static void op_54();
static void op_55();
static void op_56();
static void op_57();
static void op_58();
static void op_59();
static void op_5A();
static void op_5B();
static void op_5C();
static void get_character_data();
static void set_character_data();
static void op_60();
static void test_player_property();
static void op_63();
static void op_66();
static void op_69();
static void op_6A();
static void op_6C();
static void op_6D();
static void op_6F();
static void op_71();
static void op_73();
static void op_74();
static void op_75();
static void op_76();
static void op_77();
static void set_msg();
static void op_7A();
static void read_header_bytes(void); // 7B
static void op_7C();
static void op_7D();
static void advance_cursor(); // op_80
static void op_81();
static void op_82();
static void op_83();
static void op_84();
static void op_85();
static void load_word3AE2_resource();
static void op_wait_escape();
static void wait_event();
static void op_8A();
static void op_8B();
static void prompt_no_yes();
static void op_8D();
static void op_90();
static void op_91();
static void op_92();
static void op_93();
static void op_94();
static void op_95();
static void op_96();
static void op_97();
static void op_98();
static void op_99();
static void op_9A();
static void op_9B();
static void op_9D();

struct op_call_table {
  void (*func)();
  const char *src_offset;
};

struct op_call_table targets[] = {
  { set_word_mode, "0x3B18" },
  { set_byte_mode, "0x3B0E" },
  { NULL, "0x3B1F" },
  { op_03, "0x3B2F" },
  { op_04, "0x3B2A" },
  { op_05, "0x3B3D" },
  { op_06, "0x3B4A" },
  { op_07, "0x3B52" },
  { op_08, "0x3B59" },
  { set_word3AE2_arg, "0x3B67" },
  { load_word3AE2_gamestate, "0x3B7A" },
  { op_0B, "0x3B8C" },
  { op_0C, "0x3BA2" },
  { op_0D, "0x3BB7" },
  { op_0E, "0x3BD0" },
  { op_extract_resource_data, "0x3BED" }, // op_0F
  { op_10, "0x3C10" },
  { op_11, "0x3C2D" },
  { op_12, "0x3C59" },
  { set_gamestate_offset, "0x3C72" },
  { op_14, "0x3C8F" },
  { op_15, "0x3CAB" },
  { op_16, "0x3CCB" },
  { store_data_into_resource, "0x3CEF" },
  { op_18, "0x3D19" },
  { op_19, "0x3D3D" },
  { op_1A, "0x3D5A" },
  { NULL, "0x3D73" },
  { op_1C, "0x3D92" },
  { op_1D, "0x4ACC" },
  { NULL, "0x01B2" },
  { NULL, "0x4AF6" },
  { NULL, "0x0000" },
  { op_21, "0x3DAE" },
  { op_22, "0x3DB7" },
  { op_23, "0x3DC0" },
  { op_24, "0x3DD7" },
  { inc_byte_word_3AE4, "0x3DE5" }, // op_25
  { op_26, "0x3DEC" },
  { op_27, "0x3E06" },
  { op_28, "0x3E14" },
  { NULL, "0x3E1B" },
  { op_2A, "0x3E36" },
  { op_2B, "0x3E45" },
  { NULL, "0x3E4C" },
  { op_right_shift, "0x3E67" }, // op_2D
  { NULL, "0x3E6E" },
  { op_2F, "0x3E75" },
  { op_30, "0x3E9D" },
  { op_31, "0x3EC1" },
  { op_32, "0x3EEB" },
  { NULL, "0x3F11" },
  { op_34, "0x3F4D" },
  { op_35, "0x3F66" },
  { NULL, "0x3F8C" },
  { NULL, "0x3FAD" },
  { op_38, "0x3FBC" },
  { op_39, "0x3FD4" },
  { op_3A, "0x3FEA" },
  { NULL, "0x4002" },
  { op_3C, "0x4018" },
  { op_3D, "0x4030" },
  { op_3E, "0x4051" },
  { op_3F, "0x4067" },
  { op_40, "0x4074" },
  { op_41, "0x407C" },
  { op_42, "0x4085" },
  { op_43, "0x408E" },
  { op_jz, "0x4099" },
  { op_jnz, "0x40A3" },
  { op_js, "0x40AF" },
  { op_47, "0x40B8" },
  { op_48, "0x40ED" },
  { loop, "0x4106" },
  { op_4A, "0x4113" },
  { op_stc, "0x4122" },
  { op_4C, "0x412A" },
  { op_4D, "0x4132" },
  { NULL, "0x414B" },
  { op_4F, "0x4155" },
  { NULL, "0x4161" },
  { op_51, "0x418B" },
  { op_52, "0x41B9" },
  { op_53, "0x41C0" },
  { op_54, "0x41E1" },
  { op_55, "0x41E5" },
  { op_56, "0x41FD" },
  { op_57, "0x4215" },
  { op_58, "0x4239" },
  { op_59, "0x41C8" },
  { op_5A, "0x3AEE" },
  { op_5B, "0x427A" },
  { op_5C, "0x4295" },
  { get_character_data, "0x42D8" }, // 0x5D
  { set_character_data, "0x4322" }, // 0x5E
  { NULL, "0x4372" }, // 0x5F
  { op_60, "0x438B" },
  { test_player_property, "0x43A6" },
  { NULL, "0x43BF" },
  { op_63, "0x43F7" },
  { NULL, "0x446E" },
  { NULL, "0x44B8" },
  { op_66, "0x40C1" },
  { NULL, "0x44CB" },
  { NULL, "0x450A" },
  { op_69, "0x453F" },
  { op_6A, "0x4573" },
  { NULL, "0x45A1" },
  { op_6C, "0x45A8" },
  { op_6D, "0x45F0" },
  { NULL, "0x45FA" },
  { op_6F, "0x4607" },
  { NULL, "0x4632" },
  { op_71, "0x465B" },
  { NULL, "0x46B6" },
  { op_73, "0x47B7" },
  { op_74, "0x47C0" },
  { op_75, "0x47D1" },
  { op_76, "0x47D9" },
  { op_77, "0x47E3" },
  { set_msg, "0x47EC" },
  { NULL, "0x47FA" },
  { op_7A, "0x4801" },
  { read_header_bytes, "0x482D" },
  { op_7C, "0x4817" },
  { op_7D, "0x483B" },
  { NULL, "0x4845" },
  { NULL, "0x486D" },
  { advance_cursor, "0x487F" },
  { op_81, "0x48C5" },
  { op_82, "0x48D2" },
  { op_83, "0x48EE" },
  { op_84, "0x4907" },
  { op_85, "0x4920" },
  { load_word3AE2_resource, "0x493E" },
  { NULL, "0x4955" },
  { op_wait_escape, "0x496D" },
  { wait_event, "0x4977" },
  { op_8A, "0x498E" },
  { op_8B, "0x499B" },
  { prompt_no_yes, "0x49A5" },
  { op_8D, "0x49D3" },
  { NULL, "0x0000" },
  { NULL, "0x49DD" },
  { op_90, "0x49E7" },
  { op_91, "0x49F3" },
  { op_92, "0x49FD" },
  { op_93, "0x4A67" },
  { op_94, "0x4A6D" },
  { op_95, "0x4894" },
  { op_96, "0x48B5" },
  { op_97, "0x42FB" },
  { op_98, "0x4348" },
  { op_99, "0x40E7" },
  { op_9A, "0x3C42" },
  { op_9B, "0x416B" },
  { NULL, "0x4175" },
  { op_9D, "0x4181" },
  { NULL, "0x492D" },
  { NULL, "0x4AF0" },
  { NULL, "0x8A06" },
  { NULL, "0xE80E" },
  { NULL, "0x513A" },
  { NULL, "0x36FF" },
  { NULL, "0x3ADB" },
  { NULL, "0x36FF" },
  { NULL, "0x3AEC" },
  { NULL, "0x2689" },
  { NULL, "0x3AEC" },
  { NULL, "0xE8A2" },
  { NULL, "0xA23A" },
  { NULL, "0x3AEA" },
  { NULL, "0xE853" },
  { NULL, "0x0FE5" },
  { NULL, "0x325E" },
  { NULL, "0xA2C0" },
  { NULL, "0x3AE1" },
  { NULL, "0xE3A2" },
  { NULL, "0xEB3A" },
  { NULL, "0x8B04" },
  { NULL, "0xDB36" },
  { NULL, "0x8E3A" },
  { NULL, "0xDD06" },
  { NULL, "0x263A" },
  { NULL, "0x32AC" },
  { NULL, "0x8BE4" },
  { NULL, "0xD1D8" },
  { NULL, "0xFFE3" },
  { NULL, "0x60A7" },
  { NULL, "0x0039" },
  { NULL, "0x0000" },
  { NULL, "0x0000" },
  { NULL, "0x0000" },
  { NULL, "0x0000" },
  { NULL, "0x0000" },
  { NULL, "0x0000" },
  { NULL, "0x0000" },
  { NULL, "0x0000" },
  { NULL, "0x0000" },
  { NULL, "0x268B" },
  { NULL, "0x3AEC" },
  { NULL, "0x068F" },
  { NULL, "0x3AEC" },
  { NULL, "0x068F" },
  { NULL, "0x3ADB" },
  { NULL, "0xA258" },
  { NULL, "0x3AE8" },
  { NULL, "0xEAA2" },
  { NULL, "0xE83A" },
  { NULL, "0x0F9D" },
  { NULL, "0xC032" },
  { NULL, "0xE1A2" },
  { NULL, "0xA23A" },
  { NULL, "0x3AE3" },
  { NULL, "0xC307" },
  { NULL, "0x2688" },
  { NULL, "0x3AE3" },
  { NULL, "0x2688" },
  { NULL, "0x3AE1" },
  { NULL, "0xB7EB" },
  { NULL, "0x06C6" },
  { NULL, "0x3AE1" },
  { NULL, "0xEBFF" },
  { NULL, "0xA0B0" },
  { NULL, "0x3AEA" },
  { NULL, "0x8B4C" },
  { NULL, "0x88EC" },
  { NULL, "0x0046" },
  { NULL, "0xA5EB" },
  { NULL, "0xE8A0" },
  { NULL, "0xEB3A" },
  { NULL, "0x8BF3" },
  { NULL, "0x8AEC" },
  { NULL, "0x0046" },
  { NULL, "0xA244" },
  { NULL, "0x3AEA" },
  { NULL, "0x66E8" },
  { NULL, "0xEB0F" },
  { NULL, "0x2692" },
  { NULL, "0x8BAC" },
  { NULL, "0x8AD8" },
  { NULL, "0x6087" },
  { NULL, "0xA238" },
  { NULL, "0x3AE4" },
  { NULL, "0x85EB" },
  { NULL, "0xAC26" },
  { NULL, "0xE4A2" },
  { NULL, "0xE93A" },
  { NULL, "0xFF7D" },
  { NULL, "0x2688" },
  { NULL, "0x3AE4" },
  { NULL, "0x76E9" },
  { NULL, "0x26FF" },
  { NULL, "0x8BAC" },
  { NULL, "0xA0D8" },
  { NULL, "0x3AE4" }
};


static void push_byte(uint8_t val)
{
  cpu.sp--;
  if (cpu.sp == UINT8_MAX) {
    cpu.sp = STACK_SIZE - 1;
  }
  cpu.stack[cpu.sp] = val;
}

static void push_word(uint16_t val)
{
  push_byte((val & 0xFF00) >> 8);
  push_byte((val & 0xFF));
}

static uint8_t pop_byte()
{
  uint8_t val = cpu.stack[cpu.sp++];
  if (cpu.sp >= STACK_SIZE)
    cpu.sp = 0;

  return val;
}

static uint16_t pop_word()
{
  uint16_t val = pop_byte();
  val += pop_byte() << 8;

  return val;
}

static uint16_t peek_word()
{
  uint16_t val = cpu.stack[cpu.sp];
  val += (cpu.stack[cpu.sp + 1] << 8);

  return val;
}

// 0x3B18
static void set_word_mode()
{
  printf("%s - setting 3AE1 to 0xFF\n", __func__);
  byte_3AE1 = 0xFF;
}

// 0x3B0E
static void set_byte_mode()
{
  printf("%s - setting 3AE1 to 0\n", __func__);
  word_3AE2 &= 0xFF;
  byte_3AE1 = 0;
}

// 0x4AA1
static void populate_3ADD_and_3ADF(void)
{
  running_script = resource_get_by_index(word_3AE8);
  word_3ADF = resource_get_by_index(word_3AEA);
}

// 0x3B2F
static void op_03()
{
  // pops the stack, 1 byte.
  uint8_t al = pop_byte();
  cpu.ax = (cpu.ax & 0xFF00) | al;
  word_3AEA = al;
  populate_3ADD_and_3ADF();
}

// 0x3B2A
static void op_04(void)
{
  uint8_t al = (word_3AE8 & 0x00FF);
  cpu.ax = (cpu.ax & 0xFF00) | al;

  push_byte(al);
}

// Populate word_3AE4 with game_state[arg1]
static void op_05(void)
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;
  al = game_state.unknown[cpu.bx];
  cpu.ax = (cpu.ax & 0xFF00) | al;
  word_3AE4 = al;
}

// 0x3B4A
// op_06 (1 byte operand)
// loads loop counter.
static void op_06()
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  word_3AE4 = al;
}

// 0x3B52
static void op_07()
{
  // Pretty much always 0.
  word_3AE4 = (cpu.ax & 0xFF00) >> 8;
}

// 0x3B59
static void op_08(void)
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;
  al = word_3AE4;
  set_game_state(__func__, cpu.bx, al);
}

// 0x3B67
// op_09
static void set_word3AE2_arg(void)
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  word_3AE2 = al;
  if (byte_3AE1 != (cpu.ax >> 8))
  {
    // set high byte
    al = *cpu.pc++;
    cpu.ax = (cpu.ax & 0xFF00) | al;
    word_3AE2 = (al << 8) | (word_3AE2 & 0xFF); // XXX ? Correct.
  }
}

// 0x3B7A
// op_0A
static void load_word3AE2_gamestate(void)
{
  uint8_t al, ah;
  uint8_t gs_idx;

  gs_idx = *cpu.pc++;

  // mov ax, [bx + game_state]
  al = game_state.unknown[gs_idx];
  ah = game_state.unknown[gs_idx + 1];
  ah = ah & byte_3AE1; // mask if in byte mode.
  word_3AE2 = (ah << 8) | al;
}

// 0x3B8C
// word_3AE2 = game_state[arg + word_3AE4];
static void op_0B()
{
  uint8_t ah, al;

  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.ax += word_3AE4;
  cpu.bx = cpu.ax;

  al = game_state.unknown[cpu.bx];
  ah = game_state.unknown[cpu.bx + 1];

  ah = ah & byte_3AE1;
  cpu.ax = ah << 8 | al;
  word_3AE2 = cpu.ax;
}

// 0x3BA2
static void op_0C()
{
  uint8_t ah;

  // es:lodsw
  cpu.ax = *cpu.pc++;
  cpu.ax += ((*cpu.pc++) << 8);
  cpu.bx = cpu.ax;

  unsigned char *src = word_3ADF->bytes;
  cpu.ax = src[cpu.bx];
  cpu.ax += (src[cpu.bx + 1]) << 8;
  ah = (cpu.ax & 0xFF00) >> 8;
  ah &= byte_3AE1;
  cpu.ax = (ah << 8) | (cpu.ax & 0xFF);
  word_3AE2 = cpu.ax;
}

// 0x3BB7
// op_0D (1 word operand)
//
// word_3AE2 = 3ADF[operand + 3AE4] & (byte_3AE1 << 8)
static void op_0D()
{
  uint8_t al, ah;

  // es:lodsw
  cpu.ax = *cpu.pc++;
  cpu.ax += ((*cpu.pc++) << 8);

  cpu.ax += word_3AE4;
  cpu.bx = cpu.ax;

  unsigned char *dest = word_3ADF->bytes;
  cpu.ax = dest[cpu.bx];
  cpu.ax += (dest[cpu.bx + 1] << 8);

  ah = (cpu.ax & 0xFF00) >> 8;
  ah = ah & byte_3AE1;
  al = cpu.ax & 0xFF;
  cpu.ax = (ah << 8) | al;
  word_3AE2 = cpu.ax;
}

// 0x3BD0
static void op_0E()
{
  uint8_t al, ah;

  al = *cpu.pc++;

  cpu.bx = game_state.unknown[al];
  cpu.bx += (game_state.unknown[al + 1] << 8);

  cpu.bx += word_3AE4;

  unsigned char *es = word_3ADF->bytes;
  cpu.ax = es[cpu.bx];
  cpu.ax += es[cpu.bx + 1] << 8;

  ah = (cpu.ax & 0xFF00) >> 8;
  ah = ah & byte_3AE1;
  cpu.ax = (ah << 8) | (cpu.ax & 0xFF);
  word_3AE2 = cpu.ax;
}

// 0x3BED
// op_0F
// Extracts data from a resource and stores to word_3AE2
static void op_extract_resource_data(void)
{
  uint8_t ah, al;
  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;

  // load di properly...
  printf("%s: BX: 0x%04X\n", __func__, cpu.bx);

  // Base offset.
  cpu.di = game_state.unknown[cpu.bx];
  cpu.di += (game_state.unknown[cpu.bx + 1] << 8);
  uint8_t bl = game_state.unknown[cpu.bx + 2];
  cpu.bx = (cpu.bx & 0xFF00) | bl;

  const struct resource *r = resource_get_by_index(cpu.bx);
  printf("%s: Resource: %d\n", __func__, r->tag);
  if (r->tag == 31) {
    printf("Loading tag 31\n");
  }


  // Additional offset.
  cpu.di += word_3AE4;

  // little endian load
  // mov ax, es:[di]
  al = r->bytes[cpu.di];
  ah = r->bytes[cpu.di + 1];
  ah = ah & byte_3AE1;
  cpu.ax = ah << 8 | al;
  word_3AE2 = (ah << 8) | al;
}

// 0x3C10
static void op_10(void)
{
  uint8_t al, ah;

  al = *cpu.pc++; // 0x41
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.di = cpu.ax;
  cpu.bx = game_state.unknown[cpu.di];
  cpu.bx += game_state.unknown[cpu.di + 1] << 8;
  al = *cpu.pc++;
  cpu.bx += al;

  unsigned char *es = word_3ADF->bytes;
  cpu.ax = es[cpu.bx];
  cpu.ax += es[cpu.bx + 1] << 8;
  ah = (cpu.ax & 0xFF00) >> 8;
  ah = ah & byte_3AE1;
  cpu.ax = (ah << 8) | (cpu.ax & 0xFF);
  word_3AE2 = cpu.ax;
}

// 0x3C2D
static void op_11(void)
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;
  uint8_t ah = (cpu.ax & 0xFF00) >> 8;
  printf("op_11: 0x%04X ah: 0x%02X\n", cpu.bx, ah);
  set_game_state(__func__, cpu.bx, ah);
  if (byte_3AE1 != ah) {
    set_game_state(__func__, cpu.bx + 1, ah);
  }
}

// 0x3C59
// sets game state (arg) with word_3AE2
static void op_12(void)
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;
  cpu.cx = word_3AE2;
  set_game_state(__func__, cpu.bx, (cpu.cx & 0xFF));
  if (byte_3AE1 != ((cpu.ax & 0xFF00) >> 8)) {
    set_game_state(__func__, cpu.bx + 1, (cpu.cx & 0xFF00) >> 8);
  }
}

// 0x3C72 (op_13)
// sets game state (arg + word_3AE4) with word_3AE2
// variant of op_12
static void set_gamestate_offset(void)
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;
  cpu.cx = word_3AE2;
  cpu.bx += word_3AE4;
  set_game_state(__func__, cpu.bx, (cpu.cx & 0x00FF));
  if (byte_3AE1 != ((cpu.ax & 0xFF00) >> 8)) {
    set_game_state(__func__, cpu.bx + 1, (cpu.cx & 0xFF00) >> 8);
  }
}

// 0x3C8F
static void op_14(void)
{
  uint16_t save_ah = (cpu.ax & 0xFF00) >> 8;

  uint16_t ax = *cpu.pc++;
  ax += *cpu.pc++ << 8;
  cpu.ax = ax;
  cpu.bx = cpu.ax;

  // es:cx
  unsigned char *dest = word_3ADF->bytes;
  uint16_t dest_offset = word_3AE2;

  dest[cpu.bx] = dest_offset;
  if (byte_3AE1 != save_ah) {
    dest[cpu.bx + 1] = (dest_offset & 0xFF00) >> 8;
  }
}

// 0x3CAB
static void op_15()
{
  uint8_t save_ah = (cpu.ax & 0xFF00) >> 8;

  cpu.ax = *cpu.pc++;
  cpu.ax += ((*cpu.pc++) << 8);
  cpu.bx = cpu.ax;

  // es:cx
  unsigned char *dest = word_3ADF->bytes;
  cpu.cx = word_3AE2;
  cpu.di = word_3AE4;
  dest[cpu.bx + cpu.di] = cpu.cx & 0xFF;
  if (byte_3AE1 != save_ah) {
    dest[cpu.bx + cpu.di + 1] = (cpu.cx & 0xFF00) >> 8;
  }
}

// 0x3CCB
static void op_16()
{
  uint8_t al;
  uint16_t index;

  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  index = cpu.ax;

  cpu.bx = game_state.unknown[index];
  cpu.bx += (game_state.unknown[index + 1] << 8);
  cpu.bx += word_3AE4;

  unsigned char *es = word_3ADF->bytes;
  cpu.cx = word_3AE2;
  es[cpu.bx] = cpu.cx & 0xFF;
  if (byte_3AE1 != ((cpu.ax & 0xFF00) >> 8)) {
    es[cpu.bx + 1] = (cpu.cx & 0xFF00) >> 8;
  }
}

// 0x3CEF (op_17)
// Stores data into resource bytes.
static void store_data_into_resource(void)
{
  uint8_t offset_idx, resource_idx;
  const struct resource *r;

  offset_idx = *cpu.pc++;

  // Base offset to write to.
  cpu.di = game_state.unknown[offset_idx];
  cpu.di += (game_state.unknown[offset_idx + 1] << 8);

  // Resource to write to (by index)
  resource_idx = game_state.unknown[offset_idx + 2];

  printf("%s  index: 0x%02X base offset: 0x%04X\n",__func__,  resource_idx, cpu.di);
  r = resource_get_by_index(resource_idx);
  cpu.di += word_3AE4;
  cpu.cx = word_3AE2;

  printf("  %s: setting byte 0x%04X\n", __func__, cpu.di);
  r->bytes[cpu.di] = (cpu.cx & 0x00FF);

  // Word mode?
  if (byte_3AE1 != ((cpu.ax & 0xFF00) >> 8)) {
    r->bytes[cpu.di + 1] = ((cpu.cx & 0xFF00) >> 8);
  }
}

// 0x3D19
static void op_18(void)
{
  uint8_t al;
  uint16_t index;

  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  index = cpu.ax;
  cpu.di = game_state.unknown[index];
  cpu.di += (game_state.unknown[index + 1] << 8);

  al = *cpu.pc++;
  cpu.di += al;

  unsigned char *es = word_3ADF->bytes;
  cpu.cx = word_3AE2;

  es[cpu.di] = cpu.cx & 0xFF;
  if (byte_3AE1 != ((cpu.ax & 0xFF00) >> 8)) {
    es[cpu.di + 1] = (cpu.cx & 0xFF00) >> 8;
  }
}

// 0x3D3D
// Copy gamestate word
static void op_19()
{
  uint8_t al = *cpu.pc++;

  // Source
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.di = cpu.ax;

  // Destination
  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;

  cpu.cx = game_state.unknown[cpu.di];
  cpu.cx += (game_state.unknown[cpu.di + 1] << 8);
  set_game_state(__func__, cpu.bx, cpu.cx & 0xFF);
  if (byte_3AE1 != ((cpu.ax & 0xFF00) >> 8)) {
    set_game_state(__func__, cpu.bx + 1, (cpu.cx & 0xFF00) >> 8);
  }
}

// 0x3D5A
static void op_1A(void)
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.di = cpu.ax;
  al = *cpu.pc++;
  set_game_state(__func__, cpu.di, al);
  cpu.ax = (cpu.ax & 0xFF00) | al;
  if (byte_3AE1 != ((cpu.ax & 0xFF00) >> 8)) {
    al = *cpu.pc++;
    set_game_state(__func__, cpu.di + 1, al);
    cpu.ax = (cpu.ax & 0xFF00) | al;
  }
}

// 0x3D92
static void op_1C()
{
  uint8_t save_ah = (cpu.ax & 0xFF00) >> 8;

  cpu.ax = *cpu.pc++;
  cpu.ax += ((*cpu.pc++) << 8);
  cpu.di = cpu.ax;
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;

  unsigned char *ds = word_3ADF->bytes;
  ds[cpu.di] = al;
  if (byte_3AE1 != save_ah) {
    al = *cpu.pc++;
    cpu.ax = (cpu.ax & 0xFF00) | al;
    ds[cpu.di + 1] = al;
  }
}

// 0x4ACC
static void op_1D(void)
{
  // memcpy 0x700 bytes to or from data_D760.
  unsigned char *src = data_D760;
  uint16_t src_offset = 0;
  unsigned char *dest = word_3ADF->bytes;
  uint16_t dest_offset = word_3AE2;
  uint8_t byte_3AE4 = (word_3AE4 & 0x00FF);
  if ((byte_3AE4 & 0x80) != 0) {
    unsigned char *tmp = src;
    src = dest;
    dest = tmp;
    uint16_t tmp_offset = src_offset;
    src_offset = dest_offset;
    dest_offset = tmp_offset;
  }
  // repe movsw (move word ds:si to es:di (si, di += 2), repeat 0x380 times.
  memcpy(dest + dest_offset, src + src_offset, 0x700);
}

// 0x3DAE
// Set byte variable.
static void op_21()
{
  uint8_t byte_3AE4 = (word_3AE2 & 0x00FF);
  word_3AE4 = (word_3AE4 & 0xFF00) | byte_3AE4;
}

// 0x3DB7
// Set word variable.
static void op_22()
{
  cpu.ax = word_3AE4;
  word_3AE2 = cpu.ax;
}

// 0x3DC0
static void op_23(void)
{
  // INCREMENT [memory]

  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.di = cpu.ax;
  set_game_state(__func__, cpu.di, game_state.unknown[cpu.di] + 1);
  if (game_state.unknown[cpu.di] == 0) {
    if (byte_3AE1 != ((cpu.ax & 0xFF00) >> 8)) {
      set_game_state(__func__, cpu.di + 1, game_state.unknown[cpu.di + 1] + 1);
    }
  }
}

// 0x3DD7
static void op_24()
{
  uint8_t ah;

  cpu.ax = word_3AE2;
  cpu.ax++;
  ah = (cpu.ax & 0xFF00) >> 8;
  ah = ah & byte_3AE1;
  cpu.ax = (ah << 8) | (cpu.ax & 0xFF);
  word_3AE2 = cpu.ax;
}

// 0x3DE5
// op_25
static void inc_byte_word_3AE4()
{
  // inc byte ptr [word_3AE4]
  uint8_t byte_3AE4 = (word_3AE4 & 0x00FF);
  byte_3AE4++;
  // No virtual flags (word_3AE6) are modified for this increment operation.
  word_3AE4 = (word_3AE4 & 0xFF00) | byte_3AE4;
}

// 0x3DEC
static void op_26(void)
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.di = cpu.ax;

  cpu.cx = game_state.unknown[cpu.di];
  cpu.cx += (game_state.unknown[cpu.di + 1] << 8);
  cpu.cx--;
  uint8_t cl, ch;
  cl = cpu.cx & 0x00FF;
  ch = (cpu.cx & 0xFF00) >> 8;
  set_game_state(__func__, cpu.di, cl);
  if (byte_3AE1 != ((cpu.ax & 0xFF00) >> 8)) {
    set_game_state(__func__, cpu.di + 1, ch);
  }
}

// 0x3E06
// Decrement word_3AE2 (with byte masked).
// Underflow could occur here.
static void op_27()
{
  uint8_t ah;

  cpu.ax = word_3AE2;
  cpu.ax--;
  ah = (cpu.ax & 0xFF00) >> 8;
  ah = ah & byte_3AE1;
  cpu.ax = (ah << 8) | (cpu.ax & 0xFF);
  word_3AE2 = cpu.ax;
}

// 0x3E14
static void op_28()
{
  uint8_t byte_3AE4 = (word_3AE4 & 0x00FF);
  byte_3AE4--;
  word_3AE4 = (word_3AE4 & 0xFF00) | byte_3AE4;
}

// 0x3E36
static void op_2A()
{
  uint8_t ah;

  cpu.ax = word_3AE2;
  cpu.ax = cpu.ax << 1;

  ah = (cpu.ax & 0xFF00) >> 8;
  ah = ah & byte_3AE1;
  cpu.ax = (ah << 8) | (cpu.ax & 0xFF);
  word_3AE2 = cpu.ax;
}

// 0x3E45
static void op_2B()
{
  // shl byte ptr [word_3AE4], 1
  uint8_t byte_3AE4 = (word_3AE4 & 0x00FF);
  byte_3AE4 = byte_3AE4 << 1;
  word_3AE4 = (word_3AE4 & 0xFF00) | byte_3AE4;
}

// 0x3E67
// 0x2D
static void op_right_shift()
{
  word_3AE2 = word_3AE2 >> 1;
}

// 0x3E75
static void op_2F()
{
  uint8_t al;

  // shr byte [word_3AE6], 1
  cpu.cf = word_3AE6 & CARRY_FLAG_MASK;
  word_3AE6 = word_3AE6 >> 1;

  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;
  cpu.cx = game_state.unknown[cpu.bx];
  cpu.cx += (game_state.unknown[cpu.bx + 1] << 8);
  if (byte_3AE1 != (cpu.ax >> 8)) {
    uint16_t tmp = word_3AE2 + cpu.cx;
    cpu.cf = tmp << 16;
    word_3AE2 = tmp;
  } else {
    uint8_t byte_3AE2 = word_3AE2 & 0xFF;
    uint8_t tmp = byte_3AE2 + (cpu.cx & 0xFF);
    cpu.cf = tmp << 8;
    word_3AE2 = (word_3AE2 & 0xFF00) | tmp;
  }

  word_3AE6 = (word_3AE6 & 0xFF00) | (((word_3AE6 & 0xFF) << 1) | cpu.cf);
}

// 0x3E9D
static void op_30()
{
  uint8_t ah, al;

  printf("%s\n", __func__);
  // shr byte [word_3AE6], 1
  uint8_t cf = 0;
  cf = word_3AE6 & CARRY_FLAG_MASK;
  word_3AE6 = (word_3AE6 & 0xFF00) | ((word_3AE6 & 0xFF) >> 1);

  ah = ((cpu.ax & 0xFF00) >> 8);
  if (byte_3AE1 != ah) {
      // es:lodsw
      uint16_t ax = *cpu.pc++;
      ax += *cpu.pc++ << 8;
      cpu.ax = ax;

      // add [3AE2], ax
      word_3AE2 += ax;
      // rcl byte [3AE6], 1
      word_3AE6 = (word_3AE6 & 0xFF00) | (((word_3AE6 & 0xFF) << 1) | cf);
      return;
  }
  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  word_3AE2 += al;
  // rcl byte [3AE6], 1
  word_3AE6 = (word_3AE6 & 0xFF00) | (((word_3AE6 & 0xFF) << 1) | cf);
}

// 0x3EC1
static void op_31()
{
  uint8_t al, ah;
  unsigned int tmp;

  // shr byte [word_3AE6], 1
  cpu.cf = word_3AE6 & CARRY_FLAG_MASK;
  word_3AE6 = (word_3AE6 & 0xFF00) | ((word_3AE6 & 0xFF) >> 1);

  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;
  cpu.cx = game_state.unknown[cpu.bx];
  cpu.cx += game_state.unknown[cpu.bx + 1] << 8;
  ah = (cpu.ax & 0xFF00) >> 8;

  if (byte_3AE1 != ah) {
    // 0x3ED3
    tmp = word_3AE2 - cpu.cx;
    cpu.cf = (tmp & 0x10000) == 0x10000;
    word_3AE2 -= cpu.cx;
  } else {
    // 0x3EDF
    tmp = word_3AE2 - (cpu.cx & 0xFF);
    cpu.cf = (tmp & 0x100) == 0x100;
    word_3AE2 -= (cpu.cx & 0xFF);
  }
  cpu.cf = !cpu.cf;
  // rcl byte [3AE6], 1
  word_3AE6 = (word_3AE6 & 0xFF00) | (((word_3AE6 & 0xFF) << 1) | cpu.cf);
}

// 0x3EEB
static void op_32()
{
  uint8_t ah, al;
  unsigned int tmp;

  // shr byte [word_3AE6], 1
  cpu.cf = word_3AE6 & CARRY_FLAG_MASK;
  word_3AE6 = (word_3AE6 & 0xFF00) | ((word_3AE6 & 0xFF) >> 1);

  ah = ((cpu.ax & 0xFF00) >> 8);
  if (byte_3AE1 != ah) {
      // es:lodsw
      uint16_t ax = *cpu.pc++;
      ax += *cpu.pc++ << 8;
      cpu.ax = ax;

      tmp = word_3AE2 - cpu.ax;
      cpu.cf = (tmp & 0x10000) == 0x10000;

      word_3AE2 -= ax;
      cpu.cf = !cpu.cf;
      // rcl byte [3AE6], 1
      word_3AE6 = (word_3AE6 & 0xFF00) | (((word_3AE6 & 0xFF) << 1) | cpu.cf);
  } else {
    al = *cpu.pc++;
    cpu.ax = (cpu.ax & 0xFF00) | al;
    uint8_t byte_3AE2 = word_3AE2 & 0xFF;
    tmp = byte_3AE2 - al;
    cpu.cf = (tmp & 0x100) == 0x100;
    cpu.cf = !cpu.cf;
    byte_3AE2 -= al;
    word_3AE2 = (word_3AE2 & 0xFF00) | byte_3AE2;
    // rcl byte [3AE6], 1
    word_3AE6 = (word_3AE6 & 0xFF00) | (((word_3AE6 & 0xFF) << 1) | cpu.cf);
  }

}

// 0x3F23
static void sub_3F23()
{
  cpu.ax = word_3AE2;
  word_11C0 = cpu.ax;

  sub_11A0(word_11C4);
  cpu.ax = word_11C8;

  sub_3F2F();
}

static void sub_3F2F()
{
  // 0x3F2F
  // XXX: Fix this endian save
  set_game_state(__func__, 57, cpu.ax & 0xFF);
  set_game_state(__func__, 58, (cpu.ax & 0xFF00) >> 8);
  cpu.ax = word_11C6;
  set_game_state(__func__, 55, cpu.ax & 0xFF);
  set_game_state(__func__, 56, (cpu.ax & 0xFF00) >> 8);

  word_3AE2 = (word_3AE2 & 0xFF00) | (cpu.ax & 0xFF);
  if (byte_3AE1 != 0) {
    word_3AE2 = cpu.ax;
  }
}


// 0x3F4D
static void op_34()
{
  uint8_t al, ah;

  al = *cpu.pc++;
  word_11C2 = (word_11C2 & 0xFF00) | al;

  // al = ah
  al = (cpu.ax & 0xFF00) >> 8;
  ah = al;

  if (byte_3AE1 != ah) {
    al = *cpu.pc++;
  }
  word_11C2 = (al << 8) | (word_11C2 & 0xFF);
  al = ah;
  word_11C4 = (ah << 8) | al;

  sub_3F23();
}

// 0x3F66
static void op_35()
{
  uint8_t al, ah;

  al = *cpu.pc++;
  cpu.bx = al;

  cpu.ax = game_state.unknown[cpu.bx];
  cpu.ax += game_state.unknown[cpu.bx + 1] << 8;
  word_11C6 = cpu.ax;

  cpu.ax = game_state.unknown[cpu.bx + 2];
  cpu.ax += game_state.unknown[cpu.bx + 3] << 8;
  word_11C8 = cpu.ax;

  cpu.ax = word_3AE2;
  word_11C0 = cpu.ax;

  sub_11CE();
  cpu.ax = word_11CA;

  set_game_state(__func__, 0x3B, cpu.ax & 0xFF);
  set_game_state(__func__, 0x3C, (cpu.ax & 0xFF00) >> 8);

  sub_3F2F();
}


// 0x3FBC
static void op_38()
{
  uint8_t al, ah;

  ah = ((cpu.ax & 0xFF00) >> 8);
  if (byte_3AE1 != ah) {
    al = *cpu.pc++;
    ah = *cpu.pc++;
    cpu.ax = (ah << 8) | al;
    word_3AE2 = word_3AE2 & cpu.ax;
  } else {
    al = *cpu.pc++;
    cpu.ax = (cpu.ax & 0xFF00) | al;
    word_3AE2 = word_3AE2 & al;
  }
}

// 0x3FD4
static void op_39()
{
  uint8_t al, ah;

  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;

  cpu.ax = game_state.unknown[cpu.bx];
  cpu.ax += game_state.unknown[cpu.bx + 1] << 8;

  cpu.ax |= word_3AE2;
  ah = (cpu.ax & 0xFF00) >> 8;
  ah = ah & byte_3AE1;
  cpu.ax = (ah << 8) | (cpu.ax & 0xFF);
  word_3AE2 = cpu.ax;
}

// 0x3FEA
static void op_3A()
{
  uint8_t al, ah;

  if (byte_3AE1 == 0) {
    al = *cpu.pc++;
    ah = *cpu.pc++;
    cpu.ax = (ah << 8) | al;
    word_3AE2 = word_3AE2 | cpu.ax;
  } else {
    al = *cpu.pc++;
    word_3AE2 = word_3AE2 | al;
  }
}

// 0x4018
static void op_3C()
{
  uint8_t al, ah;

  if (byte_3AE1 == 0) {
    al = *cpu.pc++;
    ah = *cpu.pc++;
    cpu.ax = (ah << 8) | al;
    word_3AE2 = word_3AE2 ^ cpu.ax;
  } else {
    al = *cpu.pc++;
    word_3AE2 = word_3AE2 ^ al;
  }
}

// 0x4030
static void op_3D()
{
  uint8_t ah, al;
  uint16_t flags;

  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;
  // BX ?
  cpu.cx = word_3AE2;
  ah = (cpu.ax & 0xFF00) >> 8;

  // Set flag defaults
  cpu.cf = 0;
  cpu.zf = 0;
  if (byte_3AE1 != ah) {
    // 0x403E
    uint16_t check_value = game_state.unknown[cpu.bx];
    check_value += game_state.unknown[cpu.bx + 1] << 8;

    if (cpu.cx < check_value) {
      cpu.cf = 1;
    }
    if (cpu.cx == check_value) {
      cpu.zf = 1;
    }
  } else {
    // 0x404B
    uint8_t cl;
    cl = cpu.cx & 0xFF;
    if (cl < game_state.unknown[cpu.bx]) {
      cpu.cf = 1;
    }
    if (cl == game_state.unknown[cpu.bx]) {
      cpu.zf = 1;
    }
  }
  // flags;
  flags = 0;
  cpu.cf = !cpu.cf;

  flags |= cpu.sf << 7;
  flags |= cpu.zf << 6;
  flags |= 1 << 1; // Always 1, reserved.
  flags |= cpu.cf << 0;
  word_3AE6 = flags;
}

// 0x4051
static void op_3E(void)
{
  uint8_t ah, al;
  cpu.bx = word_3AE2;
  ah = ((cpu.ax & 0xFF00) >> 8);
  if (byte_3AE1 != ah) {
    al = *cpu.pc++;
    ah = *cpu.pc++;
    cpu.ax = (ah << 8) | al;
    cpu.cf = (cpu.bx - cpu.ax) < 0;
    cpu.zf = (cpu.bx - cpu.ax) == 0 ? 1 : 0;
  } else {
    uint8_t bl = cpu.bx & 0x00FF;
    al = *cpu.pc++;
    cpu.ax = (cpu.ax & 0xFF00) | al;
    cpu.cf = (bl - al) < 0;
    cpu.zf = (bl - al) == 0 ? 1 : 0;
  }

  // loc_4042
  cpu.cf = !cpu.cf;
  // pushf
  // pop word [3AE6]
  // We should copy:
  //    Carry flag, parity flag, adjust flag, zero flag
  //    sign flag, trap flag, interupt enable flag, direction, overflow.

  // XXX: Not correct, but maybe it's all we care about?
  word_3AE6 = 0;
  word_3AE6 |= cpu.zf << 6;
  word_3AE6 |= 1 << 1; // Always 1, reserved.
  word_3AE6 |= cpu.cf << 0;
}

// 0x4067
static void op_3F()
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;
  al = word_3AE4;

  // cmp and set flags.
  // I think these are the only flags that matter.
  cpu.zf = al == game_state.unknown[cpu.bx];
  cpu.cf = al < game_state.unknown[cpu.bx];

  // loc_4042
  cpu.cf = !cpu.cf;
  // pushf
  // pop word [3AE6]
  // We should copy:
  //    Carry flag, parity flag, adjust flag, zero flag
  //    sign flag, trap flag, interupt enable flag, direction, overflow.

  // XXX: Not correct, but maybe it's all we care about?
  word_3AE6 = 0;
  word_3AE6 |= cpu.zf << 6;
  word_3AE6 |= 1 << 1; // Always 1, reserved.
  word_3AE6 |= cpu.cf << 0;
}

// 0x4074
static void op_40(void)
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  uint8_t byte_3AE4 = (word_3AE4 & 0x00FF);
  int cf = 0;
  int zf = 0;

  cf = (byte_3AE4 - al) < 0;
  zf = (byte_3AE4 - al) == 0;
  cf = !cf;
  // pushf
  // pop word [3AE6]
  // We should copy:
  //    Carry flag, parity flag, adjust flag, zero flag
  //    sign flag, trap flag, interupt enable flag, direction, overflow.

  // XXX: Not correct, but maybe it's all we care about?
  word_3AE6 = 0;
  word_3AE6 |= zf << 6;
  word_3AE6 |= 1 << 1; // Always 1, reserved.
  word_3AE6 |= cf << 0;
}

// 0x407C
static void op_41(void)
{
  // Carry flag check
  if ((word_3AE6 & CARRY_FLAG_MASK) == 0) {
    uint16_t new_address = *cpu.pc++;
    new_address += *cpu.pc++ << 8;
    cpu.ax = new_address;
    printf("(op41)    New address: 0x%04x\n", new_address);
    cpu.pc = cpu.base_pc + new_address;
  } else {
    cpu.pc++;
    cpu.pc++;
  }
}

// 0x4085
// Opposite of op_41
static void op_42(void)
{
  // Carry flag check
  if ((word_3AE6 & CARRY_FLAG_MASK) == 0) {
    cpu.pc++;
    cpu.pc++;
  } else {
    uint16_t new_address = *cpu.pc++;
    new_address += *cpu.pc++ << 8;
    cpu.ax = new_address;
    printf("(op42)    New address: 0x%04x\n", new_address);
    cpu.pc = cpu.base_pc + new_address;
  }
}

// 0x408E
static void op_43()
{
  uint8_t al;

  al = word_3AE6;
  al &= 0x41;
  if (al == 1) {
    // 40A0 -> 41B9
    op_52();
    return;
  } else {
    // 40AA
    cpu.pc++;
    cpu.pc++;
  }
}

// 0x4099
// op_44
// Jump on non-zero flag.
static void op_jz(void)
{
  if ((word_3AE6 & ZERO_FLAG_MASK) == 0) {
    cpu.pc++;
    cpu.pc++;
    return;
  }
  uint16_t new_address = *cpu.pc++;
  new_address += *cpu.pc++ << 8;
  cpu.ax = new_address;
  printf("(%s)    New address: 0x%04x\n", __func__, new_address);
  cpu.pc = running_script->bytes + new_address;
}

// 0x40A3
// op_45
// Jump on zero flag.
static void op_jnz(void)
{
  if ((word_3AE6 & ZERO_FLAG_MASK) != 0) {
    cpu.pc++;
    cpu.pc++;
    return;
  }
  uint16_t new_address = *cpu.pc++;
  new_address += *cpu.pc++ << 8;
  cpu.ax = new_address;
  printf("(%s)    New address: 0x%04x\n", __func__, new_address);
  cpu.pc = running_script->bytes + new_address;
}

// 0x40AF
// Jump if signed (similar to js)
static void op_js()
{
  if ((word_3AE6 & SIGN_FLAG_MASK) != 0) {
    // 40A0
    op_52();
    return;
  }
  // 40AA
  cpu.pc++;
  cpu.pc++;
}

// 0x40B8
static void op_47()
{
  if ((word_3AE6 & SIGN_FLAG_MASK) == 0) {
    // 40A0
    op_52();
    return;
  }
  // 40AA
  cpu.pc++;
  cpu.pc++;
}

// 0x40ED
static void op_48()
{
  uint8_t al;

  clear_sign_flag();
  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;

  if (game_state.unknown[cpu.bx] < 0x80) {
    set_game_state(__func__, cpu.bx, game_state.unknown[cpu.bx] | 0x80);
    set_sign_flag();
  }
}

// 0x4106
// op_49
static void loop(void)
{
  // This is actually more of a LOOP function.
  // The counter is stored in word_3AE4, although it's only an 8 bit
  // counter.

  // byte decrement.
  uint8_t byte_3AE4 = (word_3AE4 & 0x00FF);
  byte_3AE4--;
  word_3AE4 = (word_3AE4 & 0xFF00) | byte_3AE4;

  if (byte_3AE4 != 0xFF) {
    uint16_t new_address = *cpu.pc++;
    new_address += *cpu.pc++ << 8;
    printf("LOOP 0x%04X  Counter: 0x%02X\n", new_address, byte_3AE4);
    cpu.pc = cpu.base_pc + new_address;
  } else {
    // 0x40AA
    cpu.pc++;
    cpu.pc++;
  }
}

// 0x4113
static void op_4A(void)
{
  // Increment byte on word.
  uint8_t byte_3AE4 = word_3AE4 & 0x00FF;
  byte_3AE4++;
  word_3AE4 = (word_3AE4 & 0xFF00) | byte_3AE4;

  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  if (al == byte_3AE4) {
    // eventually just goes to 0x40AA
    cpu.pc++;
    cpu.pc++;
  } else {
    uint16_t new_address = *cpu.pc++;
    new_address += *cpu.pc++ << 8;
    cpu.ax = new_address;
    printf("(op4A)    New address: 0x%04x\n", new_address);
    cpu.pc = cpu.base_pc + new_address;
  }
}

// 0x4122
static void op_stc()
{
  word_3AE6 |= CARRY_FLAG_MASK;
}

// 0x412A
static void op_4C()
{
  word_3AE6 &= 0xFFFE;
}

// 0x4132
// Pseudo random number generator?
static void op_4D()
{
  uint32_t mul_result;

  sub_2CF5();

  mul_result = cpu.ax * word_3AE2;
  word_3AE2 = (mul_result & 0x00FF0000) >> 16;
  if (byte_3AE1 != 0) {
    word_3AE2 = (mul_result & 0xFFFF0000) >> 16;
  }
}

// 0x4155
static void op_4F()
{
  uint8_t al;

  sub_4A79(word_3AE2);
  al = cpu.ax & 0xFF;

  al = ~al;

  uint8_t val = game_state.unknown[cpu.bx];
  val = val & al;
  set_game_state(__func__, cpu.bx, val);
}

// 0x418B
static void op_51()
{
  // offset of random encounter data...
  cpu.ax = *cpu.pc++;
  cpu.ax += *cpu.pc++ << 8;

  cpu.di = cpu.ax;
  cpu.ax = 0;

  uint8_t cl = 0xFF;
  uint8_t bl;

  // 4197 (zero out high word 3AE3)
  word_3AE2 = (word_3AE2 & 0xFF); // XXX ? Correct.
  cpu.bx = word_3AE4;
  bl = cpu.bx & 0xFF;

  unsigned char *es = word_3ADF->bytes;

  // 419E
  word_3AE2 = cpu.ax & 0xFF;
  word_3AE4 = (word_3AE4 & 0xFF00) | bl;
  while (--bl != 0xFF) {
    uint8_t al = es[cpu.di + bl];
    if (al >= word_3AE2) {
      word_3AE2 = al;
      word_3AE4 = (word_3AE4 & 0xFF00) | bl;
    }
  }
}

// 0x41B9
static void op_52(void)
{
  // CALL function ?
  // Save source index.
  // Jump to new source index.
  uint16_t new_address = *cpu.pc++;
  new_address += *cpu.pc++ << 8;
  printf("New address: 0x%04x\n", new_address);
  uint16_t existing_address = cpu.pc - cpu.base_pc;
  printf("Existing address: 0x%04x\n", existing_address);

  cpu.pc = cpu.base_pc + new_address;
}

// 0x41C0
static void op_53(void)
{
  // CALL function ?
  // Save source index.
  // Jump to new source index.
  uint16_t new_address = *cpu.pc++;
  new_address += *cpu.pc++ << 8;
  printf("New address: 0x%04x\n", new_address);
  uint16_t existing_address = cpu.pc - cpu.base_pc;
  printf("Existing address: 0x%04x\n", existing_address);

  push_word(existing_address);
  cpu.pc = cpu.base_pc + new_address;
}

// 0x41E1
static void op_54()
{
  // RET function.
  uint16_t si = pop_word();
  printf("%s SI: %04X\n", __func__, si);
  cpu.pc = cpu.base_pc + si;
}

// 0x41E5
static void op_55()
{
  uint8_t ah;

  // peek word, pop byte.
  cpu.cx = peek_word();
  pop_byte();
  word_3AE2 = (cpu.cx & 0xFF);
  ah = (cpu.ax & 0xFF00) >> 8;
  if (ah != byte_3AE1) {
    word_3AE2 = cpu.cx;
    pop_byte();
  }
}

// 0x41FD
static void op_56(void)
{
  cpu.cx = word_3AE2;

  uint8_t ah = (cpu.ax & 0xFF00) >> 8;
  if (byte_3AE1 != ah) {
    push_word(cpu.cx);
  } else {
    // store cl into stack.
    push_byte(cpu.cx & 0xFF);
  }
}

// 0x4215
static void op_57()
{
  uint8_t al;

  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  push_word(cpu.ax);

  cpu.ax = *cpu.pc++;
  cpu.ax += ((*cpu.pc++) << 8);

  word_3ADB = cpu.ax;
  al = word_3AE8;
  resource_index_release(al);
  cpu.bx = pop_word();
  cpu.bx = cpu.bx & 0xFF;

  al = 1;
  struct resource *r = resource_load(cpu.bx);
  al = r->index;
  word_3AE8 = al;
  word_3AEA = al;
  populate_3ADD_and_3ADF();
  cpu.pc = running_script->bytes + word_3ADB;
  cpu.base_pc = running_script->bytes;
}

// 0x4239
static void op_58(void)
{
  uint8_t al, dl;
  const struct resource *r;

  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;

  // tag lookup item.
  uint16_t tag_item = cpu.ax;

  cpu.ax = *cpu.pc++;
  cpu.ax += ((*cpu.pc++) << 8);
  uint16_t src_offset = cpu.ax;

  uint16_t si = cpu.pc - running_script->bytes;
  push_word(si);
  // push cs
  // pop es
  // dec sp
  // mov bp, sp
  al = word_3AE8;
  push_byte(al);
  // mov [bp], al
  //
  int found = find_index_by_tag(tag_item);
  if (found != -1) {
    // 0x4254
    r = resource_get_by_index(found);
    if (r->usage_type == 2) {
      dl = 0xFF;
    } else {
      // 0x4259
      // xor dl, dl
      // jmp 4268
      dl = 0;
    }
  } else {
    r = resource_load(tag_item);
    dl = 0xFF;
  }
  // 0x4268
  push_byte(dl); // dl
  al = r->index;
  word_3AE8 = al;
  word_3AEA = al;
  populate_3ADD_and_3ADF();
  // 3AC7
  cpu.pc = running_script->bytes + src_offset;
  cpu.base_pc = running_script->bytes;
}

// 0x128D
static void sub_128D(int index)
{
  resource_set_usage_type(index, 0x2);
}

// 0x41C8
static void op_59(void)
{
  uint8_t al, ah;
  uint16_t si;

  ah = (cpu.ax & 0xFF00) >> 8;
  if (ah != cpu.stack[cpu.sp]) {
    // 0x41CF
    al = word_3AE8;
    sub_128D(al);
  }
  cpu.ax = pop_word();
  ah = (cpu.ax & 0xFF00) >> 8;
  word_3AE8 = ah;
  word_3AEA = ah;
  populate_3ADD_and_3ADF();
  si = pop_word();
  cpu.pc = running_script->bytes + si;
  cpu.base_pc = running_script->bytes;
}

// 0x3AEE
static void op_5A(void)
{
  // mov sp, [3AEC]
  cpu.sp = saved_stack;
  saved_stack = pop_word();
  word_3ADB = pop_word();
  cpu.ax = pop_word();

  uint8_t al = cpu.ax & 0xFF;
  word_3AE8 = al;
  word_3AEA = al;
  populate_3ADD_and_3ADF();
  al = 0;
  cpu.ax = (cpu.ax & 0xFF00) | al;

  byte_3AE1 = al;
  word_3AE2 = (al << 8) | (word_3AE2 & 0xFF); // lower portion of word_3AE2 takes al.
}

// 0x427A
static void op_5B()
{
  printf("%s not complete\n", __func__);
  exit(1);
}

// 0x4295
static void op_5C(void)
{
  set_byte_mode();
  cpu.ax = *cpu.pc++;
  cpu.ax += ((*cpu.pc++) << 8);

  word_42D6 = cpu.ax;

  // mov [3ADB], si
  word_3ADB = cpu.pc - cpu.base_pc; // is this correct?

  if (game_state.unknown[0x1F] == 0) {
    // 0x42D3
    return;
  }

  uint8_t al = game_state.unknown[6];
  cpu.ax = (cpu.ax & 0xFF00) | al;
  push_word(cpu.ax);
  game_state.unknown[6] = 0;

  do {
    // 0x42B8
    cpu.bx = word_42D6;
    al = word_3AE8;
    run_script(al, word_42D6);
    game_state.unknown[6]++;
    al = game_state.unknown[6];
  } while (al < game_state.unknown[0x1F]);
  cpu.ax = pop_word();
  al = cpu.ax & 0xFF;
  game_state.unknown[6] = al;

  // jmp 0x3AC7
  cpu.pc = running_script->bytes + word_3ADB;
  cpu.base_pc = running_script->bytes;
}

// 0x42D8 (5D)
// Loads word_3AE2 with character data.
// The actual character property is read from argument.
static void get_character_data(void)
{
  int chr_idx;

  uint8_t player_number = game_state.unknown[6];
  cpu.ax = (cpu.ax & 0xFF00) | player_number;
  cpu.di = cpu.ax;
  cpu.bx = 0xC960;

  uint8_t bh = (cpu.bx & 0xFF00) >> 8;
  chr_idx = game_state.unknown[cpu.di + 0xA]; // Character offset selector ?
  bh += chr_idx;
  cpu.bx = bh << 8 | (cpu.bx & 0xFF);

  uint8_t al = *cpu.pc++; // Character property offset
  printf("%s - Player number: %d Property: %s (0x%02X)\n", __func__,
      player_number, player_property_name(al), al);
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx += cpu.ax;

  unsigned char *c960 = get_player_data_base();
  cpu.cx = c960[cpu.bx - 0xC960];

  // Property is now in cpu.cx, but is this a byte or word property?
  word_3AE2 = cpu.cx & 0xFF;
  if (byte_3AE1 != 0) {
    // 0x3AE3
    word_3AE2 = (cpu.cx & 0xFF00) | (word_3AE2 & 0xFF); // XXX ? Correct.
  }
}

// 0x4322 (5E)
// Set properties of character (value is in word_3AE2).
static void set_character_data(void)
{
  uint8_t al = game_state.unknown[6];
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.di = cpu.ax;
  game_state.unknown[cpu.di + 0x18] = (cpu.ax & 0xFF00) >> 8;
  cpu.bx = 0xC960;
  uint8_t bh = (cpu.bx & 0xFF00) >> 8;
  bh += game_state.unknown[cpu.di + 0xA];
  cpu.bx = bh << 8 | (cpu.bx & 0xFF);

  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx += cpu.ax;
  cpu.cx = word_3AE2;

  // Validation that we aren't writing outside our array.
  if ((cpu.bx - 0xC960) >= 0xE00) {
    printf("Array of data_C960 not large enough!\n");
    exit(1);
  }
  // mov [di], al
  unsigned char *c960 = get_player_data_base();
  c960[cpu.bx - 0xC960] = cpu.cx & 0xFF;
  if (byte_3AE1 != 0) {
    c960[cpu.bx - 0xC960 + 1] = (cpu.cx & 0xFF00) >> 8;
  }
}

// 0x438B
static void op_60()
{
  // 0x4A7D
  sub_4A79(word_3AE2);
  cpu.cx = game_state.unknown[6]; // Current player.

  cpu.di = cpu.cx;
  cpu.cx = 0xC960;

  uint8_t ch = (cpu.cx & 0xFF00) >> 8;
  ch += game_state.unknown[cpu.di + 0xA];
  cpu.cx = ch << 8 | (cpu.cx & 0xFF);

  cpu.di = cpu.cx;
  uint8_t al = cpu.ax & 0xFF;
  al = ~al; // not al

  unsigned char *c960 = get_player_data_base();
  // cpu.bx is property to AND.
  c960[(cpu.bx + cpu.di) - 0xC960] &= al;
}

static void set_flags()
{
  uint16_t flags = 0;

  flags |= cpu.sf << 7;
  flags |= cpu.zf << 6;
  flags |= 1 << 1; // Always 1, reserved.
  flags |= cpu.cf << 0;
  cpu.ax = flags;

  // Clear carry flag
  cpu.ax &= 0xFFFE;

  // Clear everything except old carry flag
  word_3AE6 &= 0x0001;

  // Store new flags (with old carry flag value)
  word_3AE6 |= cpu.ax;
}

// 0x43A6
static void test_player_property()
{
  // 4A7D
  sub_4A79(word_3AE2);
  cpu.cx = game_state.unknown[6];
  cpu.di = cpu.cx;

  cpu.cx = 0xC960;

  uint8_t val = game_state.unknown[cpu.di + 10];
  unsigned char *player = get_player_data(val >> 1);

  printf("%s: %s 0x%02X\n", __func__, player_property_name(cpu.bx), cpu.bx);

  // test [bx+di], al
  uint8_t test_val = player[cpu.bx];
  uint8_t test_result = test_val & (cpu.ax & 0xFF);
  cpu.cf = 0;
  cpu.sf = test_result >= 0x80;
  cpu.zf = test_result == 0;
  set_flags();
}

// 0x43F7
static void op_63()
{
  uint8_t al, ah;

  ah = (cpu.ax & 0xFF00) >> 8;
  byte_3AE1 = ah;

  // 0x3AE3
  word_3AE2 = (ah << 8) | (word_3AE2 & 0xFF);

  // es:lodsw
  cpu.ax = *cpu.pc++;
  cpu.ax += ((*cpu.pc++) << 8);

  word_4454 = cpu.ax;
  word_3ADB = cpu.pc - cpu.base_pc; // is this correct?
  byte_3867 = 0;

  // 0x440D

  cpu.bx = 0;
  cpu.bx = cpu.bx << 1;
  al = game_state.unknown[6];
  ah = 0;
  cpu.ax = al;
  cpu.di = cpu.ax;
  cpu.ax = 0xCA4C;
  ah = (cpu.ax & 0xFF00) >> 8;
  ah += game_state.unknown[cpu.di + 0xA]; // character select.
  cpu.ax = (ah << 8) | (cpu.ax & 0xFF);
  cpu.ax += get_unknown_4456(cpu.bx);
  cpu.di = cpu.ax;
  if (data_CA4C[cpu.di - 0xCA4C] != 0) {
    // 0x4430
    printf("%s: 0x4430 unimplemented\n", __func__);
    exit(1);
  }
  // 0x444C
  word_3AE6 &= 0xFFFE;
}

// 0x40C1
static void op_66(void)
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;

  cpu.zf = 0;
  cpu.cf = 0;
  cpu.sf = 0;

  cpu.cx = game_state.unknown[cpu.bx];
  cpu.cx += (game_state.unknown[cpu.bx + 1] << 8);

  // Check value of AH
  if (byte_3AE1 == (cpu.ax >> 8)) {
    uint8_t cl = cpu.cx & 0x00FF;
    if (cl == 0) {
      cpu.zf = 1;
    }
    if (cl >= 0x80) {
      cpu.sf = 1;
    }
  } else {
    if (cpu.cx == 0) {
      cpu.zf = 1;
    }
    if (cpu.cx >= 0x8000) {
      cpu.sf = 1;
    }
  }

  uint16_t flags = 0;
  flags |= cpu.sf << 7;
  flags |= cpu.zf << 6;
  flags |= 1 << 1; // Always 1, reserved.
  flags |= cpu.cf << 0;
  flags &= 0xFFFE;
  word_3AE6 &= 0x0001;
  word_3AE6 |= flags;
  cpu.ax = flags;
}

// 0x453F
static void op_69(void)
{
  cpu.bx = game_state.unknown[7];
  // shl bx, 1
  cpu.ax = game_state.unknown[6];
  cpu.di = cpu.ax;
  cpu.ax = 0xCA4C;

  uint8_t ah = (cpu.ax & 0xFF00) >> 8;
  ah += game_state.unknown[0xA + cpu.di];
  cpu.ax = ah << 8 | (cpu.ax & 0xFF);
  cpu.ax += get_unknown_4456(cpu.bx);
  cpu.di = cpu.ax;

  cpu.ax = *cpu.pc++;
  cpu.di += cpu.ax;

  cpu.ax = word_3AE2;
  // Validation that we aren't writing outside our array.
  if ((cpu.di - 0xCA4C) >= sizeof(data_CA4C)) {
    printf("Array of data_CA4C not large enough!\n");
    exit(1);
  }
  // mov [di], al
  data_CA4C[cpu.di - 0xCA4C] = cpu.ax & 0xFF;
  if (byte_3AE1 != 0) {
    data_CA4C[cpu.di - 0xCA4C + 1] = (cpu.ax & 0xFF00) >> 8;
  }

}

// 0x4AC0
static void set_sign_flag()
{
  word_3AE6 |= 0x40;
}

// 0x4AC6
static void clear_sign_flag()
{
  // Clear bit 7 (sign flag)
  word_3AE6 &= 0xBF;
}

// 0x4573
static void op_6A(void)
{
  int i;
  uint8_t val[4];

  clear_sign_flag();
  // 0x4576
  // push si
  //
  for (i = 0; i < 4; i++) {
    val[i] = *cpu.pc++;
  }
  if (game_state.unknown[0] >= val[0] &&
      game_state.unknown[1] >= val[1] &&
      val[2] >= game_state.unknown[0] &&
      val[3] >= game_state.unknown[1]) {
    set_sign_flag();
  }
  // 0x459A
}

// Register new position on map (defined in game state 0, 1)
// Input here is the direction.
// 0x45D0
static void adjust_position(uint8_t direction)
{
  switch (direction) {
  case DIRECTION_NORTH:
    set_game_state(__func__, 0, game_state.unknown[0] + 1);
    break;
  case DIRECTION_EAST:
    // 0x45E6
    set_game_state(__func__, 1, game_state.unknown[1] + 1);
    break;
  case DIRECTION_SOUTH:
    // 0x45EB
    set_game_state(__func__, 0, game_state.unknown[0] - 1);
    break;
  case DIRECTION_WEST:
    set_game_state(__func__, 1, game_state.unknown[1] - 1);
    break;
  default:
    break;
  }
}

// 0x45A8
static void op_6C(void)
{
  // push si
  // 45AC
  adjust_position(game_state.unknown[3]);
  // 45AF
  if ((game_state.unknown[0x23] & 0x2) != 0) {
    // 0x45B6
    printf("%s: 0x45B6 unimplemented\n", __func__);
    exit(1);
  }
  // 0x45CC
  // pop si
}

// 0x194A
// Returns bx, dx
static void sub_194A()
{
  uint8_t bl, dl;

  bl = 3;
  bl -= byte_1964;
  bl += byte_1960;

  // row?
  cpu.bx = (cpu.bx & 0xFF00) | bl;

  dl = byte_1961;
  dl -= 4;
  dl += byte_1962;

  // column?
  cpu.dx = (cpu.dx & 0xFF00) | dl;

  printf("%s: 0x%02X:0x%02X\n", __func__, bl, dl);
}

// 1A13
// Plots map segment?
// The only real input here is byte_1962
static void sub_1A13(int input)
{
  struct viewport_data vp;
  word_104F = cpu.bx;
  //word_1051 = // dragon.com 0x695C ?

  vp.xpos = input << 5;
  vp.ypos = 0x18;
  vp.data = ui_get_minimap_viewport();

  // CF8
  sub_CF8(ui_get_minimap_viewport(), &vp);
}

// 0x1861
static void sub_1861(uint8_t input)
{
  uint8_t al, bl, dl;

  sub_194A();
  al = 0;
  bl = cpu.bx & 0xFF;
  dl = cpu.dx & 0xFF;
  if (bl == game_state.unknown[0] && dl == game_state.unknown[1]) {
    al = 0xFF;
  }

  // 0x1874
  byte_1966 = al;
  sub_54D8(dl, bl);

  // 0x187A: Correct?
  if ((((word_11C6 & 0xFF00) >> 8) & 0x08) != 0) {
    // 0x1881
    bl = (word_11C6 & 0xFF00) >> 8;
    bl = bl >> 4;
    bl = bl & 0x3;
    al = data_56E5[bl + 4];
    cpu.di = 0;
    sub_19C7(al);
    bl = word_11C6;
    bl = bl >> 4;
    bl = bl & 0xF;

    // 0x18AA
    if (bl != 0) {
      // 0x18AC
      al = data_56C6[bl];
      cpu.di = 6;
      sub_19C7(al);
    }
    // 0x18B6
    bl = word_11C6;
    bl = bl & 0xF;
    if (bl != 0) {
      // 0x18C0
      al = data_56C6[bl];
      cpu.di = 0xC;
      sub_19C7(al);
    }
    if ((byte_1966 & 0x80) != 0) {
      // 0x1945
      sub_1A10();
      return;
    }

    bl = (word_11C6 & 0xFF00) >> 8;
    bl &= 0x7;
    if (bl == 0) {
      // 0x1948
      return;
    }
    al = data_56E5[bl + 7];
    cpu.di = 0;
    sub_19C7(al);
    return;
  }
  // 18E4
  cpu.bx = 0x695C;
  sub_1A13(input);

  // 18EA
  al = word_11C6;
  byte_1949 = al;

  sub_194A();
  bl = cpu.bx & 0xFF;
  bl++;
  cpu.bx = (cpu.bx & 0xFF00) | bl;
  sub_54D8(dl, bl);

  // test byte [11C7], 08
  if ((((word_11C6 & 0xFF00) >> 8) & 0x08) != 0) {
    bl = byte_1949;
    bl = bl >> 4;
    bl &= 0xF;
    if (bl != 0) {
      // 0x1911
      al = data_56C6[bl];
      cpu.di = 0x6;
      sub_19C7(al);
    }
  }

  // 191B
  sub_194A();
  dl = cpu.dx & 0xFF;
  dl--;
  cpu.dx = (cpu.dx & 0xFF00) | dl;
  sub_54D8(dl, bl);

  // test byte [11C7], 08
  if ((((word_11C6 & 0xFF00) >> 8) & 0x08) != 0) {
    bl = byte_1949;
    bl &= 0xF;
    if (bl != 0) {
      // 0x1934
      al = data_56C6[bl];
      cpu.di = 0x0C;
      sub_19C7(al);
    }
    if ((byte_1966 & 0x80) == 0) {
      return;
    }
    sub_1A10();
    return;
  }

  // 193E
  // test byte [1966], 80
  if ((byte_1966 & 0x80) == 0) {
    // 1948
    return;
  }
  // 0x1945
  // jmp 0x1A10
  sub_1A10();
}

static void sub_1967(uint8_t input)
{
  uint16_t new_offset;

  ui_set_viewport_width(0x90);
  cpu.bx = input;
  //cpu.bx = cpu.bx << 1;
  // DRS:
  if (cpu.bx > 8) {
    printf("%s: %d: Need to extract more from data_1997. We don't know how big it is.\n",
           __func__, cpu.bx);
    exit(1);
  }

  new_offset = data_1997[cpu.bx];
  ui_set_viewport_offset(new_offset);
  cpu.ax = data_19A7[cpu.bx];
  cpu.ax -= new_offset;
  ui_set_viewport_height(cpu.ax);
  cpu.di = data_19B7[cpu.bx];

  // Technically in 106F
  size_t vp_offset = get_offset(cpu.di / 2);
  ui_update_viewport(vp_offset);

  ui_set_viewport_offset(0);
  ui_set_viewport_height(0x88);
  ui_set_viewport_width(0x50);
}

// 0x19C7
// Plot resource onto minimap
static void sub_19C7(uint8_t val)
{
  struct resource *r;
  struct viewport_data vp;

  if (val > 0x7F) {
    return;
  }
  cpu.bx = 0;
  word_104F = cpu.bx;

  r = data_59E4[val];
  word_1051 = r;
  cpu.ax = byte_1962;
  cpu.ax = cpu.ax << 5;

  if (cpu.di != 0) {
    printf("%s: Dump more data_19FE, cpu.di: 0x%04X\n", __func__, cpu.di);
    exit(1);
  }

  cpu.ax += data_19FE[cpu.di];
  vp.xpos = cpu.ax;
  cpu.ax = data_19FE[cpu.di + 1]; // 1A00
  vp.ypos = cpu.ax;

  cpu.bx = data_19FE[cpu.di + 2]; // 1A02

  sub_CE7(&vp, cpu.bx);
}

// 0x1A10
static void sub_1A10()
{
  printf("%s\n", __func__);
  struct viewport_data vp;

  cpu.bx = 0x6820;
  word_104F = cpu.bx;
  // word_1051 = cs
  cpu.ax = byte_1962;
  cpu.ax = cpu.ax << 5;

  vp.xpos = cpu.ax;
  vp.ypos = 0x18;
  vp.data = ui_get_data_6820();

  sub_CF8(ui_get_data_6820(), &vp);
}

// 0x17F7
// Draw the minimap
static void sub_17F7()
{
  uint8_t al;

  // mov byte [1964], 00
  byte_1964 = 0;
  byte_104E = 0;

  // 1801: jmp short 1806
  // 1803: call 184B

  do {

    // 0x1806
    al = 0;
    do {
      byte_1962 = al;
      sub_1861(al);

      al = byte_1962;
      al++;
    } while (al < 9);

    sub_1967(byte_1964);

    // Check for key in the buffer.
    // if no key (jmp 1837)

    // 0x1820 (a key)
    cpu.cx = 4;
    cpu.di = 0x1843;
    // test al, al (key press)
    // je (0x1831)
    // 0x182A
    cpu.di = 0x1847;
    al = al & 0xDF;
    cpu.ax = (al << 8) | al;

    // 0x1831
    al = cpu.ax >> 8;
    // repne scasb
    // searching es:di for al, di++
    // je if found ret.




    // 0x1837
    byte_1964++;
    al = byte_1964;
    ui_viewport_reset(); // called 8 times?

  } while (al < 8);
}

// 0x1750
static void sub_1750()
{
  uint8_t al;
  cpu.bx = 0x17D9;

  // Values embedded in COM file at 0x17D9
  draw_rectangle(1, 0, 39, 192);
  al = game_state.unknown[0];
  byte_1960 = al;
  al = game_state.unknown[1];
  byte_1961 = al;

  // 1764
  cpu.ax = 0;
  init_offsets(0x90);
  cache_resources();
  while (1) {
    // 0x176A
    sub_176A();

    // 0x1775 jmp near bx
    if (cpu.bx == 0x179B) {
      mini_map_escape();
      return;
    } else if (cpu.bx == 0x17A7) {
      mini_map_left();
    } else if (cpu.bx == 0x17B3) {
      mini_map_right();
    } else if (cpu.bx == 0x17C0) {
      mini_map_down();
    } else if (cpu.bx == 0x17CC) {
      mini_map_up();
    } else {
      printf("%s: 0x1775 unimplemented 0x%04X\n", __func__, cpu.bx);
      exit(1);
    }
  }
}

// Not a function really, but a lot of code jumps here
// 0x176A
static void sub_176A()
{
  // data_1777 to 179A
  unsigned char data_1777[] = {
    0x80, 0x80,
    0x9B, 0x9B, 0x17, // ESC -> 0x179B
    0x88, 0xA7, 0x17, // Left -> 0x17A7
    0xCA, 0xA7, 0x17, // 'J' -> 0x17A7
    0x95, 0xB3, 0x17, // Right -> 0x17B3
    0xCC, 0xB3, 0x17, // 'L' -> 0x17B3
    0x8A, 0xC0, 0x17, // Down -> 0x17C0
    0xDA, 0xC0, 0x17, // 'Z' -> 0x17C0
    0xCB, 0xC0, 0x17, // 'K' -> 0x17C0
    0x8B, 0xCC, 0x17, // Up -> 0x17CC
    0xC1, 0xCC, 0x17, // 'A' -> 0x17CC
    0xC9, 0xCC, 0x17, // 'I' -> 0x17CC
    0xFF
  };
  unsigned char *ptr = data_1777;

  // 0x176A
  sub_17F7();

  // 0x176D
  cpu.bx = 0x1777;
  // cpu.cx = cs
  sub_28B0(&ptr, data_1777);
}

// 0x179B
static void mini_map_escape()
{
  init_offsets(0x50);
  sub_37C8();
  sub_587E(); // Redraw header?
  ui_draw_full();
}

// 0x17A7
static void mini_map_left()
{
  if (byte_1961 != 0) {
    byte_1961--;
  }
}

// 0x17B3
static void mini_map_right()
{
  // Right boundary check
  if (byte_1961 < game_state.unknown[33]) {
    byte_1961++;
  }
}

// 0x17C0
static void mini_map_down()
{
  if (byte_1960 != 0) {
    byte_1960--;
  }
}

// 0x17CC
static void mini_map_up()
{
  // up boundary check.
  if (byte_1960 < game_state.unknown[34]) {
    byte_1960++;
  }
}

// 0x45F0
// mini map
static void op_6D()
{
  unsigned char *base_pc = cpu.base_pc;

  sub_1750();
}

// 0x4607
static void op_6F(void)
{
  uint8_t al;

  cpu.dx = (cpu.dx & 0xFF00) | game_state.unknown[1];
  cpu.bx = (cpu.bx & 0xFF00) | game_state.unknown[0];
  sub_536B(cpu.dx, cpu.bx);

  al = *cpu.pc++;
  cpu.ax = al;
  cpu.di = cpu.ax;
  cpu.bx = 0;

  // 0x4620
  // copy contents of 11CA, 11CB, 11CC into game_state.
  set_game_state(__func__, cpu.di++, word_11CA & 0xFF);
  set_game_state(__func__, cpu.di++, (word_11CA & 0xFF00) >> 8);
  set_game_state(__func__, cpu.di++, (word_11CC & 0xFF));
}


// 0x46A1
static void sub_46A1(uint8_t start_offset)
{
  uint8_t al;
  uint16_t offset = start_offset;
  // 0x46A3
  offset += data_5A04[0];

  // Determine script execution src offset
  cpu.bx = data_5521[offset];
  cpu.bx += data_5521[offset + 1] << 8;
  al = game_state.unknown[0x56];
  cpu.ax = (cpu.ax & 0xFF00) | al;
  // 0x3AA0
  run_script(al, cpu.bx); // run level script?
}


// 0x465B
static void op_71(void)
{
  uint8_t al, bl, dl;
  unsigned char *base_pc = cpu.base_pc;

  uint16_t si = cpu.pc - cpu.base_pc; // is this correct?
  push_word(si);

  al = game_state.unknown[2];
  if (al == game_state.unknown[0x57]) {
    // 0x4667
    dl = game_state.unknown[1];
    bl = game_state.unknown[0];

    cpu.dx = (cpu.dx & 0xFF00) | dl;
    cpu.bx = (cpu.bx & 0xFF00) | bl;
    sub_54D8(cpu.dx, cpu.bx);

    al = word_11C8;
    if (al != game_state.unknown[0x3E]) {
      // 0x467B
      set_game_state(__func__, 0x3E, 0);
      if (al != 0) {
        // 0x4684
        set_game_state(__func__, 0x3F, al);
        cpu.ax = (cpu.ax & 0xFF00) | al;
        cpu.ax++;
        al = cpu.ax & 0xFF;
        al = al << 1;

        bl = al;
        cpu.bx = (cpu.bx & 0xFF00) | bl;
        sub_46A1(bl);

        al = game_state.unknown[2];
        if (al != game_state.unknown[0x57]) {
          return;
        }
      }
      // 0x4698
    }
    // 0x4698
    cpu.bx = 0;
    sub_46A1(0);
  }
  // 0x469D
  // pop si ?
  si = pop_word();
  cpu.base_pc = base_pc;
  cpu.pc = base_pc + si;
}

// 0x47B7
static void op_73(void)
{
  uint8_t al;

  al = game_state.unknown[0x3F];
  set_game_state(__func__, 0x3E, al);
}

// 0x47C0
static void op_74(void)
{
  uint16_t x, y, w, h;

  // draws frame, reads next 4 bytes to get rectangle dimensions.
  x = *cpu.pc++;
  y = *cpu.pc++;
  w = *cpu.pc++;
  h = *cpu.pc++;
  draw_rectangle(x, y, w, h);
}

// 0x47D1
static void op_75(void)
{
  ui_draw_full();
}

// 0x47D9
static void op_76(void)
{
  draw_pattern(&draw_rect);
}

// 0x47E3
static void op_77()
{
  draw_pattern(&draw_rect);
  set_msg();
}

// Extracts a string (likely for drawing on the screen)
// 0x47EC
static void set_msg(void)
{
  cpu.bx = extract_string(cpu.base_pc, cpu.pc - cpu.base_pc, handle_byte_callback);
  cpu.pc = cpu.base_pc + cpu.bx;
}

// 0x4801
static void op_7A()
{
  cpu.bx = extract_string(word_3ADF->bytes, word_3AE2, handle_byte_callback);
  word_3AE2 = cpu.bx;
}

// 0x1A40
// Write character name
static void write_character_name()
{
  uint8_t al, ah;

  cpu.bx = game_state.unknown[6];
  cpu.ax = 0xC960;
  ah = (cpu.ax & 0xFF00) >> 8;
  uint8_t val = game_state.unknown[cpu.bx + 10];
  ah += val;
  cpu.ax = (ah << 8) | (cpu.ax & 0xFF);
  cpu.bx = cpu.ax;

  unsigned char *player = get_player_data(val >> 1);
  printf("%s: 0x%04X, Player number: 0x%02X\n", __func__, cpu.bx, val);
  while (1) {
    al = *player++;
    ah = al;
    cpu.ax = al;
    al = al | 0x80;
    handle_byte_callback(al);
    if ((ah & 0x80) == 0) {
      break;
    }
  }
}

// 0x4817
// Random encounter, this will set the ui header.
static void op_7C(void)
{
  // si
  unsigned char *dest = word_3ADF->bytes;
  uint16_t dest_offset = word_3AE2;
  printf("%s - 0x%04X\n", __func__, dest_offset);
  set_ui_header(dest, dest_offset);
  word_3AE2 = cpu.bx;
}

// 0x483B
static void op_7D(void)
{
  printf("op_7D\n");
  write_character_name();
}

static void sub_1BE6()
{
  int counter;
  counter = cpu.ax & 0xFF;

  counter -= draw_point.x;
  if (counter <= 0)
    return;

  // Append spaces
  cpu.ax = 0xA0;
  for (int i = 0; i < counter; i++) {
    handle_byte_callback(0xA0);
  }
}

// 0x487F
// OP 80 (takes 1 argument)
static void advance_cursor(void)
{
  uint8_t al;

  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;

  ui_draw_string();
  al += draw_rect.x;
  cpu.ax = (cpu.ax & 0xFF00) | al;

  sub_1BE6();
}

// 0x1DCA
static void sub_1DCA(uint8_t dl)
{
  uint8_t al, bl;
  unsigned int tmp;

  bl = dl;
  cpu.bx = bl;
  byte_1E20 = 0;
  int counter = cpu.bx << 1;

  // 1DD4 (loop)
  while (counter >= 0) {
    dl = 0xB0;

    while (1) {
      // 1DD6
      cpu.ax = word_11C6;
      uint16_t data_val = data_1E21[counter];
      data_val += (data_1E21[counter + 1] << 8);
      tmp = cpu.ax - data_val;
      cpu.cf = (tmp & 0x10000) == 0x10000;

      cpu.ax = tmp & 0xFFFF;
      cpu.cx = cpu.ax;
      cpu.ax = word_11C8;

      data_val = data_1E21[counter + 0x14];
      data_val += data_1E21[counter + 0x15] << 8;
      tmp = cpu.ax - data_val - cpu.cf;
      cpu.cf = (tmp & 0x10000) == 0x10000;
      cpu.ax = tmp & 0xFFFF;

      if (cpu.cf == 0) {
        // 0x1DE8
        word_11C8 = cpu.ax;
        word_11C6 = cpu.cx;
        dl++;
      } else {
        // 0x1DF3
        break;
      }
    }

    int skip_1E10 = 0;

    // 0x1DF3
    if (counter != 0) {
      // 0x1DF7
      if (dl != 0xB0) {
        byte_1E20 = dl;
      }
      // 0x1E00
      if (byte_1E20 == 0) {
        if (byte_1E1F != 0) {
          dl = 0xA0;
        } else {
          skip_1E10 = 1;
        }
      }
    }
    // 0x1E10
    if (skip_1E10 == 0) {
      al = dl;
      handle_byte_callback(al);
    }
    // 0x1E15
    counter = counter - 2;
  }
  // 0x1E19
  byte_1E1F = 0;
  cpu.bx = 0;

}

// 0x1DBB
// also called by 0x1DB9 (with high val of 0)
static void sub_1DBB(uint16_t val)
{
  uint8_t dl;

  word_11C6 = val;
  dl = 4;
  word_11C8 = 0;

  sub_1DCA(dl);
}

// 0x1DC8
static void sub_1DC8()
{
  sub_1DCA(9);
}

// 0x48C5
static void op_81()
{
  cpu.ax = word_3AE2;
  sub_1DBB(cpu.ax);
}

// 0x48D2
static void op_82()
{
  uint8_t al;

  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;

  cpu.ax = game_state.unknown[cpu.bx];
  cpu.ax += game_state.unknown[cpu.bx + 1] << 8;

  word_11C6 = cpu.ax;

  cpu.ax = game_state.unknown[cpu.bx + 2];
  cpu.ax += game_state.unknown[cpu.bx + 3] << 8;

  word_11C8 = cpu.ax;
  sub_1DC8();
}

// 0x48EE
// Writes word_3AE2 out to screen.
// Could be 1 byte or 2 bytes depending on whether byte_3AE1 is set.
static void op_83(void)
{
  uint8_t al;
  if (byte_3AE1 != (cpu.ax >> 8)) {
    // high byte of 3AE2;
    al = ((word_3AE2 & 0xFF00) >> 8);
    handle_byte_callback(al);
  }
  al = word_3AE2;
  handle_byte_callback(al);
}

// 0x4907
static void op_84(void)
{
  struct resource *r = game_memory_alloc(word_3AE2, 1, 0xFFFF);
  cpu.ax = r->index;
  word_3AE2 = cpu.ax;
}

// 0x4920
static void op_85(void)
{
  cpu.ax = word_3AE2;
  resource_index_release(cpu.ax);
}

// 0x493E
// op_86
static void load_word3AE2_resource(void)
{
  const struct resource *r = resource_load(word_3AE2);
  cpu.ax = r->index;
  uint8_t ah = (cpu.ax & 0xFF00) >> 8;
  word_3AE2 = (ah & byte_3AE1) | (cpu.ax & 0x00FF);
}

// 0x4C07
static void sub_4C07()
{
  uint8_t al;

  al = game_state.unknown[0xBE + cpu.bx];
  cpu.ax = (cpu.ax & 0xFF00) | al;
  if (al != 0) {
    if (al == word_4C31[cpu.bx]) {
      cpu.zf = 1;
    }
    word_4C31[cpu.bx] = al;
    cpu.cf = 0;
    return;
  }
  // 0x4C19
  if (word_4C31[cpu.bx] == 0) {
    cpu.cf = 1;
  }
}

// 0x4B60
static void sub_4B60()
{
  uint8_t al;

  if (ui_rect_redraw(9) == 1) {
    return;
  }
  // 0x4B68
  cpu.bx = 0;
  sub_4C07();
  if (cpu.cf == 0 && cpu.zf == 0) {
    // 0x4B71
    al = cpu.ax & 0xFF;
    al += 9;
    cpu.ax = (cpu.ax & 0xFF00) | al;
    sub_35A0(cpu.ax & 0xFF);
  }
  // 0x4B76
  cpu.bx = 1;
  sub_4C07();
  if (cpu.cf == 0) {
    if (cpu.zf == 1) {
      // 0x4B80
      printf("%s: 0x4B80 unimplemented\n", __func__);
      exit(1);
    }
    // 0x4B91
    printf("%s: 0x4B91 unimplemented\n", __func__);
    exit(1);
  }
  // 0x4BA7
  cpu.bx = 2;
  sub_4C07();
  if (cpu.cf == 0 && cpu.zf == 0) {
    // 0x4BB1
    printf("%s: 0x4BB1 unimplemented\n", __func__);
    exit(1);
  }
  // 0x4BB6
  cpu.bx = 3;
  sub_4C07();
  if (cpu.cf == 0) {
    if (cpu.zf == 0) {
      // 0x4BC0
      printf("%s: 0x4BC0 unimplemented\n", __func__);
      exit(1);
    } else {
      // 0x4BC4
      printf("%s: 0x4BC4 unimplemented\n", __func__);
      exit(1);
    }
    // 0x4BDC
    printf("%s: 0x4BDC unimplemented\n", __func__);
    exit(1);
  }
  // 0x4BE7
}

// 0x4D37
static void sub_4D37(int al, int index, const struct resource *r)
{
  int cl = al;
  int si = index;
  uint8_t dl;

  si = si << 1;

  cpu.dx = r->bytes[si];
  cpu.dx += (r->bytes[si + 1]) << 8;
  dl = cpu.dx & 0xFF;

  cpu.ax = cpu.dx;

  if (cpu.dx != 0) {
    data_4F19[index + 8] = cl;   // bx + 4F21
    dl = 0xFF; // also set cpu.dx ?
  }
  data_4F19[index + 12] = dl; // bx + 4F25
  data_4F19[index * 2] = cpu.ax & 0xFF;
  data_4F19[(index * 2) + 1] = (cpu.ax & 0xFF00) >> 8;
}

// 0x4D5C
static void sub_4D5C()
{
  if (timers.timer2 != 0)
    return;

  if (byte_4F2B == 0)
    return;

  // 4D6A
  timers.timer2 = 2;
  cpu.ax = (cpu.ax & 0xFF00) | 0x0A;
  ui_rect_redraw(10);

  for (int index = 3; index >= 0; index--) {
    sub_4D97(index);
  }
}

// 0x4D97
static void sub_4D97(uint16_t index)
{
  uint8_t al, ah;
  uint16_t start_index;

  // 4D97
  if (data_4F19[index + 12] == 0) { // 4F25
    return;
  }
  // 4D9E
  if (data_4F19[index + 8] != 0) { // 4F21
    data_4F19[index + 8]--;
    return;
  }

  // 0x4DAA
  // push old bx (but it was index)
  cpu.bx = index << 1;
  start_index = data_4F19[cpu.bx];
  start_index += data_4F19[cpu.bx + 1] << 8;
  // 0x4DB1
  cpu.si = start_index + 1;

  cpu.bx = word_4F29->bytes[cpu.si];
  cpu.bx += word_4F29->bytes[cpu.si + 1] << 8;

  sub_4DE3(cpu.bx, word_4F29);
  vga_update();
  // 0x4DC0

  al = word_4F29->bytes[start_index];
  ah = word_4F29->bytes[start_index + 3];
  if (ah == 0xFF) {
    sub_4D37(al, index, word_4F29);
    // Jump to 4D37
    return;
  }
  // 0x4DD7
  data_4F19[index + 8] = al; // load 0x4F21
  cpu.di = start_index;
  cpu.di += 3;
  cpu.ax = cpu.di;
  // jmp 0x4D55

  data_4F19[index * 2] = cpu.ax & 0xFF;
  data_4F19[(index * 2) + 1] = (cpu.ax & 0xFF00) >> 8;
}

// 0x1A68
void reset_game_state()
{
  memset(game_state.unknown + 0x18, 0, 7);

  sub_1A72();
}

// 0x1A72
static void sub_1A72()
{
  uint8_t al, ah;
  uint8_t val;

  if (ui_rect_redraw(11) == 1) {
    return;
  }

  al = draw_point.x;
  ah = draw_point.y;
  cpu.ax = (ah << 8) | al;
  push_word(cpu.ax);
  cpu.ax = (ah << 8) | game_state.unknown[6];
  push_word(cpu.ax);

  // 0x1A85
  set_sb_handler_ui_draw_chr();
  int counter = 6;
  while (counter >= 0) {
    // 0x1A8B
    al = game_state.unknown[0x18 + counter];
    if (al < 0x80) {
      // 0x1A93

      // Set currently selected player.
      set_game_state(__func__, 6, counter & 0xFF);
      ah = 0;
      // si = ax

      if (al == 0) {
        val = 0x10;
      } else {
        val = 0x01;
      }
      al = val;
      sub_1ABD(al);
      set_game_state(__func__, 0x18 + counter, 0xFF);
    }
    counter--;
  }
  cpu.ax = pop_word();
  set_game_state(__func__, 6, cpu.ax & 0xFF);
  cpu.ax = pop_word();
  draw_point.y = (cpu.ax & 0xFF00) >> 8;
  draw_point.x = cpu.ax & 0xFF;
  set_sb_handler_append_string();
}

// 0x1ABD
// input will be 0x01 or 0x10
static void sub_1ABD(uint8_t val)
{
  uint8_t al, ah, bl;
  uint16_t fill_color; // 3713
  int si;

  cpu.ax = (cpu.ax & 0xFF00) | val;
  cpu.bx = cpu.ax;
  bl = cpu.bx & 0xFF;
  bl = bl & 0xF; // make sure it's not higher than 0xF.
  byte_1BE5 = bl; // color.

  ui_set_background(cpu.ax);

  bl = game_state.unknown[0x6];
  al = bl;
  al = al << 4;
  al += 0x20;
  draw_point.y = al;
  draw_point.x = 0x1B;
  cpu.bx = bl; // xor bh, bh

  cpu.ax = 0xC960;
  ah = (cpu.ax & 0xFF00) >> 8;

  ah += game_state.unknown[cpu.bx + 0xA]; // Character selector ?
  cpu.ax = (ah << 8) | (cpu.ax & 0xFF);
  player_base_offset = cpu.ax;

  al = game_state.unknown[6];
  if (al >= game_state.unknown[31]) {
    // 1AF6
    fill_color = byte_1BE5; // color ?
    al = draw_point.y;
    cpu.ax = al;
    g_linenum = cpu.ax; // line number
    word_36C0 = 0x36; // starting point
    word_36C2 = 0x4E; // ending point

    while (1) {
      ui_draw_solid_color(fill_color, word_36C0, word_36C2, g_linenum);
      g_linenum++;

      if ((g_linenum & 0x0F) == 0) {
        break;
      }
    }
    reset_ui_background();
    return;
  }

  // 1B22
  al = 0xC;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  unsigned char *c960 = get_player_data_base();
  unsigned char *di = c960 + (player_base_offset - 0xC960);
  di--;

  // 1B29
  do {
    di++;
    cpu.ax--;
  } while ((*di & 0x80) == 0x80);

  // 1B30
  al = cpu.ax & 0xFF;
  uint8_t carry = al & 0x1;
  al = al >> 1;
  al = al + 0x1B + carry; // adc al, 0x1B
  cpu.ax = (cpu.ax & 0xFF00) | al;
  sub_1BE6();
  write_character_name();
  al = 0x27;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  sub_1BE6();
  di = c960 + (player_base_offset - 0xC960);
  al = di[0x4C];

  si = 3;

  // 1B4A
  int found = 0;
  while (si >= 0) {
    uint8_t si_val = get_1BC1_table(si);
    if ((si_val & al) != 0) {
      found = 1;
      break; // 1B95
    }
    si--;
  }

  if (found == 0) {
    // 1B53 (not found)
    uint8_t dl = 2; // health
    cpu.bx = 0x14;
    al = 8;
    cpu.ax = (cpu.ax & 0xFF00) | al;
    sub_1BF8(dl, al);
    // 0x1B5D
    dl = 3; // stun
    cpu.bx = 0x18;
    al = 0x0B;
    cpu.ax = (cpu.ax & 0xFF00) | al;
    sub_1BF8(dl, al);
    dl = 4; // magic
    cpu.bx = 0x1C;
    al = 0x0E;
    sub_1BF8(dl, al);
    al = byte_1BE5;
    fill_color = al;
    g_linenum--;
    word_36C0 = 0x36;
    word_36C2 = 0x4E;
    // 0x1B87
    ui_draw_solid_color(fill_color, word_36C0, word_36C2, g_linenum);
    g_linenum -= 3;
    ui_draw_solid_color(fill_color, word_36C0, word_36C2, g_linenum);
    reset_ui_background();
    return;
  }
  // 1B95
  printf("%s 0x1B95 %d %d unimplemented\n", __func__, si, found);
  exit(1);
}

// 0x1F10
static void sub_1F10()
{
  if (mouse.enabled == 0) {
    return;
  }

  printf("%s: 0x1F17 unimplemented\n", __func__);
  exit(1);
}

// 0x2CF5
// Get timer ticks?
static void sub_2CF5()
{
  cpu.ax = sys_ticks();
  cpu.ax += random_seed;
  random_seed = cpu.ax;
}

// 0x3824
static void poll_mouse()
{
  // No support for reading mouse position at this point.
  // This is determined by 0x3855.
  cpu.ax = 0;

  mouse.enabled = 0;
  mouse.x = 0;
  mouse.y = 0;
  mouse.clicked = 0;
}

// 0x2AEE
// Check if mouse is inbounds on a rectangle?
static int sub_2AEE()
{
  word_246D = 2;
  cpu.ax = mouse.x;
  cpu.ax = cpu.ax << 3;

  if (cpu.ax > draw_rect.x) {
    printf("%s: 0x2BO2 unimplemented\n", __func__);
    exit(1);
  }

  if ((word_2AA7 & 0x04) != 0) {
    cpu.ax = cpu.ax & 0xFF00;
    word_246D = 0;
    cpu.cf = 1;
    return 1;
  }
  if ((word_2AA7 & 0x10) != 0) {
    cpu.ax = mouse.x;
    if (cpu.ax >= 0xD8) {
      // 0x2B4B
      printf("%s: 0x2B4B unimplemented\n", __func__);
      exit(1);
    }
  }
  // 0x2B81
  if ((word_2AA7 & 0x20) != 0) {
    cpu.ax = mouse.x;
    if (cpu.ax >= 0x10) {
      printf("%s: 0x2B8E unimplemented\n", __func__);
      exit(1);
    }
  }

  // 0x2BCF
  return 0;
}

// 0x3840
static uint8_t mouse_get_clicked()
{
  // Mouse clicked will store the last 2 clicks in the high bits
  // of mouse.clicked.
  return mouse.clicked & 0xC0;
}

// 0x2D0B
// returns keycode in AL, also sets sign flag?
static uint16_t sub_2D0B()
{
  cpu.bx = word_2DD9;
  if (cpu.bx < 0x8000) {
    printf("%s: 0x2D13 unimplemented\n", __func__);
    exit(1);
  }
  // 0x2D31
  do {
    cpu.ax = vga_getkey();
    if (cpu.ax == 0) {
      return cpu.ax;
    }
    if (cpu.ax == 0x93) {
      // Ctrl-S (turn on/off sound)
      printf("xor byte_107, 0x40\n");
    }
  } while (cpu.ax == 0x93);

  // 0x2D4B
  // We have a key now.
  cpu.bx = word_2DD7;
  if (cpu.bx >= 0x8000) {
    // cpu.bx is signed (negative value)
    return cpu.ax;
  }
  // 0x2D53

  printf("%s: 0x2D53 unimplemented\n", __func__);
  exit(1);

  return cpu.ax;
}

// 0x2BD9
static int sub_2BD9()
{
  sub_4D5C();
  sub_4B60();
  sub_1A72();
  cpu.ax = timers.timer3; // 0x4C38
  uint8_t al = cpu.ax & 0xFF;
  al = al | timers.timer5; // 0x4C3C
  cpu.ax = (cpu.ax & 0xFF00) | al;
  if (cpu.ax != 0) {
    return 0;
  }
  timers.timer3 = 0xB4;
  al = game_state.unknown[5];
  al++;
  al = al & 0x1F;
  game_state.unknown[5] = al;
  return 1;
}

// 0x2ADC
static void sub_2ADC()
{
  if ((word_2AA7 & 0x1) != 0) {
    return;
  }
  zero_out_2AAA();
}

// 0x2A4C
// Inputs byte_2AA6 ?
static void sub_2A4C(const unsigned char *base)
{
  uint8_t al;

  cpu.di++;
  printf("%s: 2AA6-2AA7 (0x%02X/0x%04X) DI 0x%04X\n", __func__,
    byte_2AA6, word_2AA7, cpu.di);

  // Actual jump address
  cpu.bx = *(base + cpu.di);
  cpu.bx += *(base + cpu.di + 1) << 8;

  printf("%s: JUMP (BX) - 0x%04X\n", __func__, cpu.bx);
  printf("%s: AX - 0x%04X\n", __func__, cpu.ax);

  sub_2ADC();
  al = cpu.ax & 0xFF;
  if (al == 1) {
    // Set selected player
    al = byte_2AA6;
    printf("%s: AL - 0x%02X\n", __func__, al);

    al -= 0xB1; // '1' & 0x80
    game_state.unknown[0x6] = al;
  }
  al = byte_2AA6;
  cpu.ax = (cpu.ax & 0xFF00) | al;
}

// 0x1F8F
static void sub_1F8F()
{
  if (byte_2476 == 0)
    return;

  printf("%s: 0x1F96 unimplemented\n", __func__);
  exit(1);
}

// 0x4B10
static void timer_tick_proc()
{
  if (timers.timer2 > 0) {
    timers.timer2--;
  }

  if (timers.timer4 > 0) {
    timers.timer4--;
  }
}

// 0x28B0
// The inputs here have to do with the keys we accept.
// Inputs:
//    BX: offset from source pointer.
// Side effect, advances src_ptr (in/out variable)
// Returns:
//    AX: key pressed.
//    BX: offset to jump to, within current script.
//
// First two bytes are some sort of flag, 0x8000 indicates that
// we draw an escape table
static void sub_28B0(unsigned char **src_ptr, unsigned char *base)
{
  uint8_t al, ah;
  uint8_t bl, bh;

  ui_draw_string();

  // Read flags (word)
  // It is currently unknown what the first 2 bytes (word)
  // does in this function.
  cpu.ax = **src_ptr;
  (*src_ptr)++;
  cpu.ax += **src_ptr << 8;
  (*src_ptr)++;

  bl = cpu.bx & 0xFF;

  // 0x28BA
  word_2AA7 = cpu.ax;

  // cpu.ax &= 20FF
  ah = (cpu.ax & 0xFF00) >> 8;
  ah = ah & 0x20;
  al = cpu.ax & 0xFF;
  cpu.ax = (ah << 8) | al;

  printf("%s: 2AA7: 0x%04X AX: 0x%04X\n", __func__, word_2AA7, cpu.ax);

  timers.timer5 = ah;
  // extract 0x2AA8
  al = (word_2AA7 & 0xFF00) >> 8;
  al = al & 0x10;
  if (al != 0) {
    al = **src_ptr;
    (*src_ptr)++;
  }
  byte_2AA9 = al;
  word_2AA2 = *src_ptr - base; // is this correct? (si)

  word_2AA4 = *src_ptr;

  if ((word_2AA7 & 0x80) != 0) {
    sub_1F8F();
  }
  // 0x28E4
  if ((word_2AA7 & 0x8000) != 0) {
    // 0x28EB
    al = draw_rect.h;
    al -= 8;
    draw_point.y = al;
    // 0x32BF, 0x32C1, 0x80
    ui_set_byte_3236(0);

    bl = (word_2AA7 & 0xFF00) >> 8;
    cpu.bx = (cpu.bx & 0xFF00) | bl;
    cpu.bx = cpu.bx & 0x3;
    al = draw_rect.w;
    al -= draw_rect.x;
    al -= escape_string_table[cpu.bx];
    if (al <= 0) {
      al = 0;
    }
    al = al >> 1;
    al += draw_rect.x;
    draw_point.x = al;
    cpu.bx = cpu.bx << 1;
    bl = cpu.bx & 0xFF;
    printf("%s: bl = 0x%02X\n", __func__, bl);
    // mov bx, [bx + 0x2A6C]
    cpu.bx = escape_string_table[bl + 4];
    cpu.bx += escape_string_table[bl + 5] << 8;

    printf("%s: cpu.bx = 0x%04X\n", __func__, cpu.bx);

    cpu.bx = extract_string(escape_string_table, (cpu.bx - 0x2A68), handle_byte_callback);
    ui_draw_string();

    bl = draw_point.y;
    bl -= draw_rect.y;
    bl = bl >> 3;
    al = 0xFF;
    data_2AAA[bl] = 0xFF;
    data_2AAA[bl + 25] = 0x9B; // ESC?
  }

  // 0x2942
  sub_4D5C();
  sub_4B60();
  sub_1A72();
  printf("%s: word_2AA7: 0x%04X\n", __func__, word_2AA7);

  // 0x294B:
  while (1) {
    if ((word_2AA7 & 0x0080) == 0) {
      sub_1F10();
    }
    sub_2CF5(); // timer
    poll_mouse(); // mouse ?
    sub_2AEE(); // Mouse in bounds?
    uint8_t clicked = mouse_get_clicked();
    if (clicked == 0x80) {
      printf("%s: 0x2965 unimplemented\n", __func__);
      exit(1);
    }

    // 0x2985
    al = sub_2D0B(); // key pressed?
    if (al != 0) {
      // A-Z letters.
      if (al >= 0xE1 && al <= 0xFA) {
        // 0x2992
        if ((word_2AA7 & 0x2) == 0) {
          al = al & 0xDF;
        }
      }

      // 0x299B
      // All other keys
      printf("%s: word_2AA7: 0x%04X\n", __func__, word_2AA7);
      if ((word_2AA7 & 0x8000) != 0) {
        if ((word_2AA7 & 0x4000) == 0) {
          if (al == 0xA0) {
            al = 0x9B; // Treat space bar as escape?
          }
        }
      }
    } else {

      // dragon.com invokes a system tick timer that runs 18.2 times per second
      // This simulates that effect.
      timer_tick_proc();
      sys_delay(1000/18);

      // 0x29B1
      if (sub_2BD9() == 0) {
        // No event occurred, keep looping.
        continue;
      }
      // 0x29B6
      al = 1;
    }

    // 29B8
    byte_2AA6 = al;
    if ((word_2AA7 & 0x40) != 0) {
      sub_2ADC();
      cpu.bx = word_2AA2;
      return;
    }

    // 0x29CC
    // ax = word_2AA4
    cpu.di = word_2AA2;
    cpu.di -= 3;
    *src_ptr -= 3;

    uint8_t dl = byte_2AA6;
    // 0x29DD
    // Loop through all possible key presses
    while (1) {
      cpu.di += 3;
      al = *(base + cpu.di);
      cpu.ax = (cpu.ax & 0xFF00) | al;
      if (al == 0) {
        sub_2A4C(base);
        return;
      }
      if (al == 0xFF) {
        break;
      }

      // 0x29EF
      // Numeric key
      if (al == 0x01) {
        bl = dl;
        bl -= 0xB1; // '1' + 0x80

        // Did we press a key to corresponds to a member of our party.
        if (bl < game_state.unknown[0x1F]) {
          // 0x29FE
          bh = 0;
          cpu.bx = bl;
          uint16_t si = cpu.bx;
          cpu.bx = 0xC960;
          bh = game_state.unknown[0xA + si];
          cpu.bx += (bh << 8);
          unsigned char *c960 = get_player_data_base();
          // Check player's status, see if alive still ?
          cpu.cx = c960[cpu.bx - 0xC960 + 0x4C];
          cpu.cx = cpu.cx & byte_2AA9; // Status modifier?
          if (cpu.cx != 0) {
            // Player is not alive.
            continue;
          }

          sub_2A4C(base);
          return;
        }
      }
      // 0x2A15
      else if (al == 0x02) {
        printf("%s: 0x2A19 unimplemented\n", __func__);
        exit(1);
      }
      // 0x2A20
      else if (al != 0x80) {

        // 0x2A24
        if (al == 0x81) {
          cpu.di++;
        } else {
          // 0x2A2B
          if ((al & 0x80) == 0) {
            // 0x2A2F
            cpu.di++;
            al = al | 0x80;
            cpu.ax = (cpu.ax & 0xFF00) | al;
            if (al > byte_2AA6)
            {
              continue;
            }
            // 0x2A38
            bl = *(base + cpu.di);
            bl |= 0x80;
            if (bl < byte_2AA6) {
              // 0x29DD
              continue;
            }
            sub_2A4C(base);
            return;
          }
          // Did user press this key??
          if (al == byte_2AA6) {
            // 0x2A4C
            sub_2A4C(base);
            return;
          }
        }
      }
    }
  }
}

#if 0
static int sub_294B()
{
  uint8_t al;
  uint8_t bl, bh;

  if ((word_2AA7 & 0x0080) == 0) {
    sub_1F10();
  }
  sub_2CF5(); // timer
  poll_mouse(); // mouse ?
  sub_2AEE(); // Mouse in bounds?
  uint8_t clicked = mouse_get_clicked();
  if (clicked == 0x80) {
    printf("%s: 0x2965 unimplemented\n", __func__);
    exit(1);
  }

  // 0x2985
  al = sub_2D0B(); // key pressed?
  if (al != 0) {
    // A-Z letters.
    if (al >= 0xE1 && al <= 0xFA) {
      // 0x2992
      if ((word_2AA7 & 0x2) == 0) {
        al = al & 0xDF;
      }
    }

    // 0x299B
    // All other keys
    printf("%s: word_2AA7: 0x%04X\n", __func__, word_2AA7);
    if ((word_2AA7 & 0x8000) != 0) {
      if ((word_2AA7 & 0x4000) == 0) {
        if (al == 0xA0) {
          al = 0x9B; // Treat space bar as escape?
        }
      }
    }
  } else {
    // 0x29B1
    if (sub_2BD9() == 0) {
      // No event occurred, keep looping.
      return 1;
    }
    // 0x29B6
    al = 1;
  }

  // 29B8
  byte_2AA6 = al;
  if ((word_2AA7 & 0x40) != 0) {
    sub_2ADC();
    cpu.bx = word_2AA2;
    return 0;
  }

  // 0x29CC
  // ax = word_2AA4
  cpu.di = word_2AA2;
  cpu.di -= 3;
  *src_ptr -= 3;

  uint8_t dl = byte_2AA6;
  // 0x29DD
  // Loop through all possible key presses
  while (1) {
    cpu.di += 3;
    al = *(base + cpu.di);
    cpu.ax = (cpu.ax & 0xFF00) | al;
    if (al == 0) {
      sub_2A4C();
      return 0;
    }
    if (al == 0xFF) {
      break;
    }

    // 0x29EF
    // Numeric key
    if (al == 0x01) {
      bl = dl;
      bl -= 0xB1; // '1' + 0x80

      // Did we press a key to corresponds to a member of our party.
      if (bl < game_state.unknown[0x1F]) {
        // 0x29FE
        bh = 0;
        cpu.bx = bl;
        uint16_t si = cpu.bx;
        cpu.bx = 0xC960;
        bh = game_state.unknown[0xA + si];
        cpu.bx += (bh << 8);
        unsigned char *c960 = get_player_data_base();
        // Check player's status, see if alive still ?
        cpu.cx = c960[cpu.bx - 0xC960 + 0x4C];
        cpu.cx = cpu.cx & byte_2AA9; // Status modifier?
        if (cpu.cx != 0)
          continue;

        sub_2A4C();
        return 0;
      }
    }
    // 0x2A15
    else if (al == 0x02) {
      printf("%s: 0x2A19 unimplemented\n", __func__);
      exit(1);
    }
    // 0x2A20
    else if (al != 0x80) {

      // 0x2A24
      if (al == 0x81) {
        cpu.di++;
      } else {
        // 0x2A2B
        if ((al & 0x80) == 0) {
          // 0x2A2F
          cpu.di++;
          al = al | 0x80;
          cpu.ax = (cpu.ax & 0xFF00) | al;
          if (al > byte_2AA6)
          {
            continue;
          }
          // 0x2A38
          bl = *(base + cpu.di);
          bl |= 0x80;
          if (bl < byte_2AA6) {
            // 0x29DD
            continue;
          }
          sub_2A4C();
          return 0;
        }
        // Did user press this key??
        if (al == byte_2AA6) {
          // 0x2A4C
          sub_2A4C();
          return 0;
        }
      }
    }
  }
}
#endif

// 0x2C00
// Takes a pointer?
static void sub_2C00()
{
  // 0x2C0E
  // Acceptable keys:
  // Flags: 0x8204, only 1 acceptable key: 0x9B (Escape)
  unsigned char data_2C0E[] = { 0x04, 0x82, 0x9B, 0x00, 0x00, 0xFF };

  cpu.bx = 0x2C0E; // function pointer.
  unsigned char *ptr = data_2C0E;
  sub_28B0(&ptr, data_2C0E);
  draw_pattern(&draw_rect);
}

// 0x496D
// Waits for an escape key before advancing.
static void op_wait_escape()
{
  sub_2C00();
}

// 0x4977
// Waits on an event to occur. This takes an unknown number of parameters
// until reading a 0xff character.
static void wait_event(void)
{
  printf("%s : 0x4977\n", __func__);

  // offset to start at.
  word_3ADB = cpu.pc - running_script->bytes;
  cpu.base_pc = running_script->bytes;
  cpu.bx = word_3ADB;

  sub_28B0(&cpu.pc, cpu.base_pc);

  // 0x4984 (A good idea to break here so you can trap keypresses).
  // the key pressed will be in AX (OR'd with 0x80).
  cpu.ax = cpu.ax & 0x00FF;
  printf("%s: BX: 0x%04X\n", __func__, cpu.bx);
  cpu.pc = cpu.base_pc + cpu.bx;
  word_3AE2 = cpu.ax; // key pressed
}

// 0x4C40
// Random Encounter!
static void sub_4C40()
{
  uint8_t bl;
  struct resource *r, *r2;

  if ((cpu.ax & 0xFF) == byte_4F0F) {
    // No monster, 0xFF is not a monster.
    return;
  }

  // 0x4C47 (al contains random encounter id)
  byte_4F0F = (cpu.ax & 0xFF);
  sub_4D82();
  bl = byte_4F0F;
  if (bl < 0xFE) {
    cpu.bx = bl;
    cpu.bx = cpu.bx << 1;
    cpu.bx += 0x8A;
    //al = 1;

    // BX will contain the monster's graphic to load

    // XXX TEMPORARY BEGIN
    //cpu.bx = 0xA8; // Load a specific resource
    // XXX TEMPORARY END

    printf("Loading Resource: %d\n", cpu.bx);
    r = resource_load(cpu.bx);
    if (r != NULL) {
      sub_4C95(r);
      sub_128D(r->index);

      uint16_t tag = r->tag;
      tag++;

      r2 = resource_load(tag);
      if (r2 == NULL) {
        // 4C91
        return;
      }

      // 4F10 = r2->index;
      // 4F29 = r2->bytes
      word_4F29 = r2;
      for (int bx = 3; bx >= 0; bx--) {
        sub_4D37(0, bx, r2);
      }
      byte_4F2B = 0xFF;
      return;
    }
  }
  // 0x4C92 ?
  sub_37C8();
}

// 0x498E
static void op_8A()
{
  cpu.ax = word_3AE2; // Random encounter ?
  sub_4C40();
}

// 0x1EBF
static void sub_1EBF()
{
  int zf = 0;
  uint8_t al;

  uint16_t flags = 0;
  flags |= zf << 6;
  flags |= 1 << 1; // Always 1, reserved.
  flags |= cpu.cf << 0;
  push_word(flags);

  al = draw_rect.w;
  al -= draw_rect.x;

  al = al - (2 + cpu.cf);
  cpu.ax = (cpu.ax & 0xFF00) | al;
  push_word(cpu.ax);

  if (al >= 0x10) {
    al = 0x10;
  }
  byte_1F08 = al;
  cpu.ax = pop_word();
  al = cpu.ax & 0xFF;
  al -= byte_1F08;
  al = al >> 1;
  al += draw_rect.x;
  draw_point.x = al;
  al = 0xBA;
  ui_draw_chr_piece(al);
  if (byte_1F07 != 0) {
    // 0x1EED
    cpu.bx = 0;
    while (cpu.bx < byte_1F07) {
      al = game_state.unknown[cpu.bx + 0xC6];
      ui_draw_chr_piece(al);
      cpu.bx++;
    }
  }
  // 0x1EFD
  flags = pop_word();
  al = 0xA0; // Space
  if (cpu.cf == 0) {
    al = 0xFE;
  }
  ui_draw_chr_piece(al);
}

// 0x1EBB
static void sub_1EBB()
{
  // clc
  cpu.cf = 0;
  sub_1EBF();
}

// 0x1EBE
static void sub_1EBE()
{
  cpu.cf = 1;
  sub_1EBF();
}

// 0x1E49
static void sub_1E49()
{
  uint8_t al, bl;

  ui_draw_string();
  byte_1F07 = 0;
  sub_1EBB();

  // 0x1E54
  while (1) {
    cpu.bx = 0x1EB9; // function pointer.
    unsigned char *ptr = data_1EB9;
    sub_28B0(&ptr, data_1EB9);

    // Checking for keys.
    al = cpu.ax & 0xFF;

    // Filter out invalid keys.
    if (al == 0xAF || al == 0xDC) {
      // Dragon wars doesn't like '/' or '\' characters.
      continue;
    }

    // 0x1E64
    bl = byte_1F07;
    cpu.bx = bl;
    if (al == 0x88) { // Backspace
      if (cpu.bx == 0) {
        continue;
      }
      byte_1F07--;
      sub_1EBB();
      ui_draw_chr_piece(0xA0);
      continue;
    } else if (al == 0x8D) { // Enter key
      // jmp 1E99
      break;
    } else if (al == 0x9B) { // escape.
      byte_1F07 = 0;
      break;
      // jmp 1E93
    } else if (bl >= byte_1F08) {
      continue;
    } else if (al < 0xA0) {
      continue;
    }
    if (al == 0xA0) {
      // 0x1E82
      if (cpu.bx == 0)
        continue;
        // 1E54
    } else {
      set_game_state(__func__, 0xC6 + cpu.bx, al);
      byte_1F07++;
      sub_1EBB();
    }
  }
  // 1E99
  al = 0;
  set_game_state(__func__, 0xC6 + cpu.bx, al);
  sub_1EBE();
  al = 0x8D;
  handle_byte_callback(al);
}

static void sub_1C70(unsigned char *src_ptr)
{
  cpu.bx = extract_string(src_ptr, 0, handle_byte_callback);
  cpu.cf = 0;
}

// 0x4D82
void sub_4D82()
{
  // validate that byte_4F10 is equal to 0xFF (which it is set to on startup)
  // byte_4F10 is some kind of memory/resource index flag, 0xFF means that it
  // hasn't been initialized.
  if (byte_4F10 != 0xFF) {
    sub_128D(byte_4F10);
  }
  byte_4F2B = 0;
  byte_4F10 = 0xFF;
}

// 0x5879
static void advance_data_ptr()
{
  cpu.di += cpu.bx;
  cpu.bx = 0;
}

// 0x5868
static void sub_5868(struct resource *res, int starting_index)
{
  uint8_t al;

  // Load components of the viewport, store resource indices
  // into data_56E5.
  do {
    al = res->bytes[cpu.bx + cpu.di];
    cpu.ax = (cpu.ax & 0xFF00) | al;
    push_word(cpu.ax);
    al &= 0x7F;

    data_56E5[starting_index] = al;
    cpu.bx++;
    starting_index++;
    cpu.ax = pop_word();
  } while ((cpu.ax & 0xFF) < 0x80);

  cpu.di += cpu.bx;
  cpu.bx = 0;
}

// 0x57DB
// Read's level data.
static void read_level_metadata()
{
  uint8_t al;

  // resource index.
  al = game_state.unknown[0x56];
  // test al, al
  if (al >= 0x80)
    return;

  struct resource *r = resource_get_by_index(al);
  data_5521 = r->bytes;
  data_5866 = r->bytes; // UI header.
  cpu.di = 0;

  // 0x57F0
  // Read first 4 bytes of level data.
  while (cpu.di < 4) {
    al = r->bytes[cpu.di];
    set_game_state(__func__, cpu.di + 0x21, al);
    cpu.di++;
  }
  // 0x57FF
  cpu.bx = 0;

  // Read next bytes into 0x5897 until we hit byte > 0x80
  do {
    // 0x5801
    al = r->bytes[cpu.bx + cpu.di];
    data_5897[cpu.bx] = al;
    cpu.bx++;
  } while (al < 0x80);
  advance_data_ptr();

  // 0x5810
  cpu.si = 0;
  do {
    al = r->bytes[cpu.bx + cpu.di];
    cpu.ax = (cpu.ax & 0xFF00) | al;
    push_word(cpu.ax);
    al &= 0x7F;
    cpu.ax = (cpu.ax & 0xFF00) | al;
    data_56C6[cpu.si + 1] = al; // 56C7
    cpu.bx++;
    al = r->bytes[cpu.bx + cpu.di];
    data_56C6[cpu.si + 0xf + 1] = al; // 0x56D6
    cpu.bx++;
    cpu.si++;
    cpu.ax = pop_word();
  } while ((cpu.ax & 0xFF) < 0x80);

  // 0x582B
  // cache resources. Types
  cpu.si = 0;
  sub_5868(r, 0);
  cpu.si = 4; // ground components
  sub_5868(r, 4);
  cpu.si = 8; // other ?
  sub_5868(r, 8);

  // 0x583C
  // Index of compressed level string (e.g. "Purgatory")
  cpu.ax = r->bytes[cpu.bx + cpu.di];
  cpu.ax += (r->bytes[cpu.bx + cpu.di + 1]) << 8;
  word_5864 = cpu.ax;

  cpu.di += 2;
  al = game_state.unknown[0x22]; // Level boundaries?
  cpu.ax = al;
  cpu.si = cpu.ax;
  al = game_state.unknown[0x21];
  al = al << 1;
  al += game_state.unknown[0x21];
  cpu.ax = al;

  // 0x5858
  // The final bytes loaded here are the offsets of the script?
  do {
    data_5A04[cpu.si] = cpu.di;
    cpu.di += cpu.ax;
    cpu.si--;
  } while (cpu.si < 0x8000); // jump not signed.
}

// 0x5523
// Check boundaries
static void sub_5523()
{
  uint8_t bl;

  bl = cpu.bx & 0xFF;
  if (bl < game_state.unknown[0x22]) {
    return; // 0x5558
  }
  // 0x5529
  printf("%s 0x5529 unimplemented,\n", __func__);
  exit(1);
}

// 0x5559
// Check boundaries
static void sub_5559()
{
  uint8_t dl;

  dl = cpu.dx & 0xFF;
  if (dl < game_state.unknown[0x21]) {
    return; // 0x558B
  }

  // 0x555F
  printf("%s 0x555F unimplemented,\n", __func__);
  exit(1);
}

// 0x54D8
// Take input parameters DX and BX ?
// not matching dragon.com
static void sub_54D8(int x, int y)
{
  uint8_t al;

  sub_5523();
  sub_5559();

  // xor dh, dh
  cpu.dx = cpu.dx & 0xFF;
  cpu.ax = cpu.dx;
  cpu.ax = cpu.ax << 1;
  cpu.ax += cpu.dx;
  cpu.bx = cpu.bx & 0xFF;
  cpu.ax += data_5A04[cpu.bx + 1]; // 0x5A06
  word_551F = cpu.ax;


  // les di, [551F]
  // di = 551F
  // ES = 5521
  cpu.di = cpu.ax;
  cpu.ax = data_5521[cpu.di];
  cpu.ax += data_5521[cpu.di + 1] << 8;
  printf("%s - DI: 0x%04X AX: 0x%04X\n", __func__, cpu.di, cpu.ax);

  word_11C6 = cpu.ax;
  al = data_5521[cpu.di + 2];
  word_11C8 = al;
  cpu.cf = 0;
  if ((byte_551E & 0x80) == 0x80) {
    // 0x550B
    cpu.ax = 0;
    byte_551E = 0;
    word_11C8 = 0;
    word_11C6 &= 0x3000;
    cpu.cf = 1;
  }
}

// 0x504B
static void sub_504B(unsigned char *dest, unsigned int offset)
{
  uint8_t al;

  al = cpu.cx & 0xFF;
  al = ~al;

  al &= data_D760[cpu.si - 0xD760];
  if (al != 0) {
    al = 8;
  }
  dest[offset + 1] &= 0xF7;
  dest[offset + 1] |= al;
  cpu.ax = (cpu.ax & 0xFF00) | al;
}


// 0x4FD9
static void sub_4FD9()
{
  uint8_t bl, cl;

  bl = game_state.unknown[2];
  word_5038 = sub_504B;
  cpu.bx = bl;
  cpu.bx = cpu.bx << 1;
  if (cpu.bx < 0x50) {
    // 0x4FEC
    cpu.si = data_D760[cpu.bx];
    cpu.si += data_D760[cpu.bx + 1] << 8;
    push_word(cpu.si);
    cpu.si &= 0x7; // make sure we aren't accessing greater than 7.
    cl = data_4A99[cpu.si];
    cl = ~cl;
    cpu.cx = (cpu.cx & 0xFF00) | cl;
    cpu.si = pop_word();
    cpu.si = cpu.si >> 3;
    cpu.si += 0xD7B0;

    cpu.bx = 0;
    cpu.dx = 0;

    // 0x500A
    // Loop through level data, populate important events?
    do {
      do {
        push_word(cpu.dx);
        push_word(cpu.bx);
        push_word(cpu.cx);
        push_word(cpu.si);

        // 0x500E
        sub_54D8(cpu.dx, cpu.bx);
        cpu.si = pop_word();
        cpu.cx = pop_word();

        // 0x5011
        // les ei, [551F]
        // di = 551F
        // ES = 5521
        cpu.di = word_551F;
        word_5038(data_5521, word_551F);

        // stc
        // rcr cl, 1
        cl = cpu.cx & 0xFF;
        cpu.cf = cl & 1;
        cl = cl >> 1;
        cl += 0x80;
        if (cpu.cf == 0) {
          cl = cl << 1;
          cpu.si++;
        }
        cpu.bx = pop_word();
        cpu.dx = pop_word();
        cpu.dx++;
      } while ((cpu.dx & 0xFF) < game_state.unknown[0x21]);
      // 0x502E
      cpu.dx = 0;
      cpu.bx++;
    } while ((cpu.bx & 0xFF) < game_state.unknown[0x22]);
  }
}

// 0x536B
// Controls movement?
static void sub_536B(int x, int y)
{
  uint8_t al, bl, dl;

  push_word(cpu.dx);
  push_word(cpu.bx);
  sub_54D8(x, y);
  cpu.bx = pop_word();
  cpu.dx = pop_word();

  word_11CC = word_11C8;
  cpu.ax = word_11C6;
  word_11CA = cpu.ax;
  // 0x537E

  if (game_state.unknown[3] != 0) {
    // 0x5385
    push_word(cpu.dx);
    push_word(cpu.bx);
    cpu.dx++;
    sub_54D8(cpu.dx, cpu.bx);
    cpu.bx = pop_word();
    cpu.dx = pop_word();

    al = word_11C6;
    al = al & 0xF;
    cpu.ax = (cpu.ax & 0xFF00) | al;
    // mov [[11CD], al
    word_11CC = (al << 8) | (word_11CC & 0xFF);

    push_word(cpu.dx);
    push_word(cpu.bx);
    cpu.bx--;
    sub_54D8(cpu.dx, cpu.bx);
    cpu.bx = pop_word();
    cpu.dx = pop_word();

    // 0x539D
    bl = word_11C6;
    bl &= 0xF0;
    bl |= ((word_11CC & 0xFF00) >> 8);
    cpu.bx = (cpu.bx & 0xFF00) | bl;
    dl = word_11CA;
    cpu.dx = (cpu.dx & 0xFF00) | dl;
    al = game_state.unknown[3];
    cpu.ax = (cpu.ax & 0xFF00) | al;
    if (al > 2) {
      // 53B5
      dl = cpu.dx & 0xFF;
      dl = dl << 4;
      bl = cpu.bx & 0xFF;
      bl = bl >> 4;
      dl = dl | bl;
      cpu.bx = (cpu.bx & 0xFF00) | bl;
      cpu.dx = (cpu.dx & 0xFF00) | dl;
      word_11CA = (word_11CA & 0xFF00) | dl;
    } else if (al == 2) {
      // 53E3
      word_11CA = (word_11CA & 0xFF00) | bl;
    } else {
      // 53CC
      bl = cpu.bx & 0xFF;
      bl = bl << 4;
      dl = cpu.dx & 0xFF;
      dl = dl >> 4;
      dl = dl | bl;
      cpu.bx = (cpu.bx & 0xFF00) | bl;
      cpu.dx = (cpu.dx & 0xFF00) | dl;
      word_11CA = (word_11CA & 0xFF00) | dl;
    }
  }
}

// 0x5764
static void sub_5764()
{
  uint8_t al, bl;

  al = game_state.unknown[2];
  if (al != game_state.unknown[0x57]) {
    // 0x576D
    al = al ^ game_state.unknown[0x57];
    if ((al & 0x7F) != 0) {
      if (game_state.unknown[0x56] != 0xFF) {
        // 0x577C
        printf("%s 0x577C unimplemented, al = 0x%02X\n", __func__, al);
        exit(1);
      }
      // 0x578A
      // Load level resource
      bl = game_state.unknown[2];
      cpu.bx = bl;
      cpu.bx += 0x46;
      al = 1;
      struct resource *r = resource_load(cpu.bx);
      set_game_state(__func__, 0x56, r->index);

      read_level_metadata();
      // 579E
      sub_4FD9();
      // 0x57A1
      cpu.cx = 2;
      cpu.di = 0x3919;
      cpu.ax = 0;
      set_game_state(__func__, 0xb9, 0);
      set_game_state(__func__, 0xba, 0);
      set_game_state(__func__, 0xbb, 0);
      set_game_state(__func__, 0xbc, 0);
    }
    // 0x57AB
    read_level_metadata();
    al = game_state.unknown[2];
    set_game_state(__func__, 4, al);
    set_game_state(__func__, 0x57, al);
  }
  // 0x57B7
  al = game_state.unknown[4];
  if (al != game_state.unknown[0x5B]) {
    // 0x57C0
    set_game_state(__func__, 0x5B, al);
    al = game_state.unknown[0x5A];
    sub_128D(al);
    bl = game_state.unknown[4];
    cpu.bx = bl;
    cpu.bx += 0x1E;
    al = 1;
    struct resource *r = resource_load(cpu.bx);
    al = r->index;
    set_game_state(__func__, 0x5A, al); // index of something.
    cpu.ax = (cpu.ax & 0xFF00) | al;
  }
}

// 0x59A6
// Caches resource indexes in data_5897
// Caches resource data in data_59E4
static void cache_resources()
{
  uint8_t al, bl;
  struct resource *r;

  cpu.bx = 0xFFFF;

  // Cache resources indexes.
  // 0x59A9
  do {
    cpu.bx++;
    push_word(cpu.bx);
    bl = data_5897[cpu.bx];
    cpu.bx = (cpu.bx & 0xFF00) | bl;
    cpu.bx &= 0x7F;
    cpu.bx += 0x6E;
    al = 1;
    r = resource_load(cpu.bx);
    cpu.bx = pop_word();
    //       [bx + 0x58A6]
    data_5897[cpu.bx + 0xf] = r->index;
    al = data_5897[cpu.bx];
  } while (al < 0x80);

  // Cache resources
  // 0x59C8
  //
  cpu.bx = 0xE;
  // 0x59CB
  do {
    //            [bx + 0x58A6]
    al = data_5897[cpu.bx + 0xf];
    if (al < 0x80) {
      // 0x59D3
      // 12A8
      r = resource_get_by_index(al);
      data_59E4[cpu.bx] = r;
    }
    // 0x59E0
    cpu.bx--;
  } while (cpu.bx != 0xFFFF);
}

// 0xCE7
static void sub_CE7(struct viewport_data *vp, uint16_t sprite_offset)
{
  unsigned char *ds = word_1051->bytes + word_104F + sprite_offset;

  cpu.ax = *ds;
  ds++;
  cpu.ax += (*ds) << 8;
  printf("%s: BX: 0x%04X AX: 0x%04X\n", __func__, sprite_offset, cpu.ax);

  if (cpu.ax == 0)
    return;

  word_104F += cpu.ax; // offset of viewport data.
  // 0xCF8
  ds = word_1051->bytes + word_104F;
  vp->data = ds;
  sub_CF8(vp->data, vp);
}

static void sub_56FC()
{
  uint8_t al, bl;
  int tmp_carry;
  struct resource *r;
  struct viewport_data vp;

  bl = data_5A56[0x1C]; // 0x5A72 ?

  // rcl 4 times
  cpu.cf = 0; // XXX, maybe not correct.
  for (int i = 0; i < 3; i++) {
    // rcl bl, 1
    tmp_carry = bl & 0x80 ? 1 : 0;
    bl = (bl << 1) + cpu.cf;
    cpu.cf = tmp_carry;
  }
  cpu.bx = (cpu.bx & 0xFF00) | bl;
  cpu.bx &= 3;

  bl = data_56E5[cpu.bx];
  cpu.bx = (cpu.bx & 0xFF00) | bl;
  al = data_5897[cpu.bx];
  al &= 0x7F;
  cpu.ax = (cpu.ax & 0xFF00) | al;

  if (al == 1) {
    // 0x5718
    // Draw sky tile?
    r = data_59E4[cpu.bx];
    word_1051 = r;
    cpu.ax = 0;
    word_104F = cpu.ax;
    vp.xpos = cpu.ax;
    word_36C0 = cpu.ax;
    g_linenum = cpu.ax;
    vp.ypos = cpu.ax;
    byte_104E = cpu.ax & 0xFF;
    cpu.bx = 4; // sprite offset

    // 5732
    sub_CE7(&vp, cpu.bx);
  } else {

    // 0x5735
    printf("%s 0x5735 unimplemented 0x%04X\n", __func__, cpu.bx);
    exit(1);
  }
}

// 0x587E
static void sub_587E()
{
  uint8_t al;
  int counter = 0xE;

  do {
    // 0x5881
    // mov al, [si + 0x58A6]
    al = data_5897[counter + 0xf];
    if (al <= 0x7F) {
      // 5889
      sub_128D(al);
    }
    // 0x588E
    data_5897[counter + 0xf] = 0xFF;
    counter--;
  } while (counter >= 0);
}

// 0x51B0
static void refresh_viewport()
{
  uint8_t al, bl, dl;
  struct resource *r;
  struct viewport_data vp;
  int counter;

  sub_4D82();

  // Load level.
  sub_5764();
  set_ui_header(data_5866, word_5864);
  cpu.bx = game_state.unknown[3];

  // Setup field of vision?
  cpu.si = data_5303[cpu.bx];
  cpu.di = 0xB;
  do {
    // 0x51D0
    push_word(cpu.di);
    push_word(cpu.si);

    dl = game_state.unknown[1];
    dl += data_530B[cpu.si];
    cpu.dx = (cpu.dx & 0xFF00) | dl;
    bl = game_state.unknown[0];
    bl += data_530B[cpu.si + 1];
    cpu.bx = bl;

    // 0x51E2
    sub_536B(dl, bl);
    cpu.si = pop_word();
    cpu.di = pop_word();
    printf("%s 0x%04X 11CA: 0x%04X\n", __func__, cpu.di, word_11CA);
    al = word_11CA;
    data_5A56[cpu.di] = al; // other components?
    al = (word_11CA & 0xFF00) >> 8;
    al &= 0xF7;
    cpu.ax = (cpu.ax & 0xFF00) | al;
    data_5A56[cpu.di + 0xC] = al; // ground
    cpu.si--;
    cpu.si--;
    cpu.di--;
  } while (cpu.di != 0xFFFF);

  bl = data_5A56[10];
  printf("%s 0x51FC BL - 0x%02X\n", __func__, bl);
  bl = bl >> 4;

  if (bl != 0) {
    // 0x520C
    cpu.bx = bl;
    bl = data_56C6[cpu.bx + 0xF]; // 0x56D5
    cpu.bx = bl;
  }
  set_game_state(__func__, 0x26, bl);
  if ((game_state.unknown[0x23] & 8) == 0 && game_state.unknown[0xc1] == 0) {
    // 0x37C8
    // draw_viewport ??
    printf("%s 0x5224 (call 37C8) unimplemented\n", __func__);
    exit(1);
  }
  // 0x5227
  dl = game_state.unknown[1];
  cpu.dx = (cpu.dx & 0xFF00) | dl;
  bl = game_state.unknown[0];
  cpu.bx = (cpu.bx & 0xFF00) | bl;
  sub_54D8(cpu.dx, cpu.bx);
  if (cpu.cf == 0) {
    // 0x5234
    data_5521[word_551F + 1] |= 0x8;
  }
  // 0x523E
  cache_resources();
  sub_56FC();

  // Components of the ground (9 sprites).
  // These are drawn in the following order:
  // 8: Top right, 7: Top left, 6: Top middle
  // 5: Middle right, 4. Middle left, 3. Middle middle
  // 2. Bottom right. 1. Bottom left, 0. Bottom middle
  // (Changing this order probably requires a change to data_56A3).
  // 0x5244
  counter = 8;
  do {
    // 0x5247
    cpu.bx = data_56A3[counter];
    bl = data_5A56[cpu.bx + 0xC]; // base 0x5A62
    bl = bl >> 4;
    cpu.bx = (cpu.bx & 0xFF00) | bl;
    cpu.bx &= 3; // 3 types of ground?

    // [bx+56E9]
    bl = data_56E5[cpu.bx + 4]; // resource index by type (ground components)
    cpu.bx = (cpu.bx & 0xFF00) | bl;

    // mov ax, [bx + 0x59E4]
    r = data_59E4[cpu.bx]; // resources used in level
    word_1051 = r;
    word_104F = 0;

    vp.xpos = ground_points[counter].x;
    vp.ypos = ground_points[counter].y;
    byte_104E = 0;

    // sprite index
    cpu.bx = sprite_indices[counter];

    sub_CE7(&vp, cpu.bx);
    counter--;
  } while (counter >= 0);

  // Other components.
  // 0x528E
  counter = 23;
  do {
    // 0x5291
    cpu.ax = data_55EF[counter];
    cpu.di = cpu.ax;
    cpu.di &= 0x007F;
    bl = data_5A56[cpu.di];
    if ((cpu.ax & 0xFF) > 0x80) {
      bl = bl >> 4;
    }
    cpu.bx = (cpu.bx & 0xFF00) | bl;
    cpu.bx &= 0x000F;
    if (cpu.bx != 0) {
      // 0x52B2
      cpu.cx = data_564F[counter];
      byte_104E = cpu.cx & 0xFF;
      al = data_56C6[cpu.bx];
      // if cpu.cx is odd.
      if (((cpu.cx & 0xFF) & 1) == 1) {
        // 0x52C3 (odd)
        al = data_56E5[cpu.bx + 0x7];
      }
      // 0x52C7
      if (al <= 0x7F) {
        // 0x52CB
        al = al << 1;
        cpu.ax = al;
        cpu.di = cpu.ax;

        // 52D1
        //
        r = data_59E4[cpu.di >> 1];
        word_1051 = r;
        word_104F = 0;
        vp.xpos = data_558F[counter];
        vp.ypos = data_55BF[counter];

        cpu.bx = data_561F[counter];
        sub_CE7(&vp, cpu.bx);
      }
    }
    // 0x52F3
    counter--;
  } while (counter >= 0);
  // 0x52F8
  //
  byte_4F0F = 0xFF; // Monster index?
  sub_587E();
  update_viewport();
}


// 0x499B
static void op_8B()
{
  // push si
  // 499B
  refresh_viewport();
}

// Prompt (N)o or (Y)es
// A Y sets carry and zero flag to 1.
// A N sets carry flag to 1 and zero flag to 0
// Any other value sets both carry and zero flag to 0.
// 0x49A5
static void prompt_no_yes()
{
  // 0x49CA (keys: 0xCE = 'N', 0xD9 = 'Y', unknown about other bytes)
  unsigned char data_49CA[] = { 0x00, 0x00, 0xCE, 0x00, 0x00, 0xD9, 0x00, 0x00, 0xFF };

  sub_1C70(data_49AB);
  cpu.bx = 0x49CA;
  unsigned char *ptr = data_49CA;
  sub_28B0(&ptr, data_49CA);
  uint8_t key = cpu.ax;

  draw_pattern(&draw_rect);

  if (key == 0xD9) { // 'Y'
    cpu.cf = 1;
    cpu.zf = 1;
  } else {
    // 'N'
    cpu.zf = 0;
    if ((key & 0xFF) > 0xD9) {
      cpu.cf = 0;
    } else {
      cpu.cf = 1;
    }
  }

  word_3AE6 = 0;
  word_3AE6 |= cpu.sf << 7;
  word_3AE6 |= cpu.zf << 6;
  word_3AE6 |= 1 << 1; // Always 1, reserved.
  word_3AE6 |= cpu.cf << 0;
}

// 0x49D3
static void op_8D()
{
  printf("%s : 0x49D3\n", __func__);
  sub_1E49();
}

static void sub_5076(int function_idx)
{
  // Output sound effect based on function.
  if (function_idx >= NUM_FUNCS) {
    printf("%s unknown function %d\n", __func__, function_idx);
    exit(1);
  }

  func_5060[function_idx]();
}

static void sub_5096()
{
  // Check sound, output sound.

  // if (byte_107, 0x80) {
  // }
}

static void sub_50B2()
{
  printf("%s: unimplemented\n", __func__);
  exit(1);
}

// Sound for opening a door?
static void sub_5080()
{
  cpu.dx = 0x28;
  cpu.bx = 0x400;
  sub_5096();
}

// Sound for ?
static void sub_5088()
{
  cpu.dx = 0xF0;
  cpu.bx = 0x200;
  sub_5096();
}

// Sound for bumping into a wall?
static void sub_5090()
{
  cpu.dx = 0xC8;
  cpu.bx = 0x800;
  sub_5096();
}

// 49E7
// Sound Movement?
static void op_90(void)
{
  uint8_t al;

  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
//  push_si
  sub_5076(al);
  //pop si
}

// 0x49F3
static void op_91(void)
{
  sub_1A72();
}

// 0x49FD
static void op_92(void)
{
  uint8_t al, dl;

  sub_1A72();
  ui_draw_string();
  cpu.bx = get_game_state(__func__, 0xDC); // 0x393C
  if (cpu.bx != 0) {
    printf("%s: 0x%02X unhandled\n", __func__, cpu.bx);
    exit(1);
  }
  cpu.ax = data_4A5B[cpu.bx];
  timers.timer4 = cpu.ax;

  while(timers.timer4 != 0) {
    poll_mouse();
    uint8_t clicked = mouse_get_clicked();
    if (clicked != 0x80) {
      // 0x4A1F
      al = sub_2D0B();
      if (al < 0x80) {
        // 0x4A24
        sub_4D5C();
        sub_4B60();
        sub_1F10();
      } else {
        // 4A38
        // mov dl, [0x393C]
        dl = get_game_state(__func__, 0xDC); // 0x393C
        if (al == 0x88) { // LEFT
          printf("%s: 0x4A40 unhandled\n", __func__);
          exit(1);
          dl--;
          if (dl < 0x80) {
            dl++;
          }
        }
        if (al == 0x95) { // RIGHT
          dl++;
          printf("%s: 0x4A4A unhandled\n", __func__);
          exit(1);
        }
        // 0x4A53
        set_game_state(__func__, 0xDC, dl);
        return;
      }
    } else {
      // 0x4A57
      return;
    }
    // XXX: HACK XXX
    // dragon.com invokes a system tick timer that runs 18.2 times per second
    // This simulates that effect.
    timer_tick_proc();
    sys_delay(1000 / 18);
    // 0x4A38
  }
  // 0x4A47
}

// 0x4A67
static void op_93(void)
{
  uint8_t byte_3AE4 = (word_3AE4 & 0x00FF);
  uint8_t al = byte_3AE4;
  cpu.ax = (cpu.ax & 0xFF00) | al;

  push_byte(al);
}

// 0x4A6D
// Who pushes ?
static void op_94(void)
{
  uint8_t al = pop_byte();
  uint8_t byte_3AE4 = al;

  word_3AE4 = (word_3AE4 & 0xFF00) | byte_3AE4;
}

// 0x4894
static void op_95()
{
  uint8_t byte_3AE4 = (word_3AE4 & 0x00FF);
  uint8_t al;

  ui_draw_string();

  al = byte_3AE4;
  al += draw_rect.y;
  draw_point.y = al;


  al = word_3AE2;
  al += draw_rect.x;
  draw_point.x = al;
  ui_set_byte_3236(al);
}

// 0x48B5
static void op_96()
{
  uint8_t al;

  ui_draw_string();
  al = draw_rect.w;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  sub_1BE6();
}

// 0x42FB
// Loads character data into word_3AE2
static void op_97()
{
  uint8_t al, ch, cl;

  al = game_state.unknown[6];
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.di = cpu.ax;

  cpu.bx = 0xC960;
  uint8_t bh = (cpu.bx & 0xFF00) >> 8;
  bh += game_state.unknown[cpu.di + 0xA]; // Character selector ?
  cpu.bx = bh << 8 | (cpu.bx & 0xFF);

  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx += cpu.ax;
  cpu.bx += word_3AE4;

  unsigned char *c960 = get_player_data_base();

  cpu.cx = c960[cpu.bx - 0xC960];
  cl = c960[cpu.bx - 0xC960];
  ch = c960[cpu.bx - 0xC960 + 1];
  cpu.cx = (ch << 8) | cl;
  word_3AE2 = cl;
  if (byte_3AE1 != (cpu.ax & 0xFF00) >> 8) {
    word_3AE2 = cpu.cx;
  }
}

// 0x4348
static void op_98()
{
  uint8_t al;

  al = game_state.unknown[6];
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.di = cpu.ax;
  set_game_state(__func__, cpu.di + 0x18, (cpu.ax & 0xFF00) >> 8);
  cpu.bx = 0xC960;

  uint8_t bh = (cpu.bx & 0xFF00) >> 8;
  bh += game_state.unknown[cpu.di + 0xA]; // Character selector ?
  cpu.bx = bh << 8 | (cpu.bx & 0xFF);

  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx += cpu.ax;
  cpu.bx += word_3AE4;
  cpu.cx = word_3AE2;

  unsigned char *c960 = get_player_data_base();
  c960[cpu.bx - 0xC960] = cpu.cx & 0xFF;
  if (byte_3AE1 != (cpu.ax & 0xFF00) >> 8) {
    c960[cpu.bx - 0xC960 + 1] = (cpu.cx & 0xFF00) >> 8;
  }
}

// 0x40E7
static void op_99(void)
{
  cpu.cx = word_3AE2;

  // XXX: Figure out what test this is really trying to do.
  // Zero flag, carry flag, sign flag?
  int zf = 0;
  int cf = 0;
  int sf = 0;

  // 40C9
  if (byte_3AE1 == (cpu.ax >> 8)) {
    uint8_t cl = cpu.cx & 0x00FF;
    if ((cl & cl) == 0) {
      zf = 1;
    }
    sf = (cl >= 0x80);
  } else {
    if ((cpu.cx & cpu.cx) == 0) {
      zf = 1;
    }
    sf = (cpu.cx >= 0x8000);
  }

  uint16_t flags = 0;
  flags |= sf << 7;
  flags |= zf << 6;
  flags |= 1 << 1; // Always 1, reserved.
  flags |= cf << 0;
  flags &= 0xFFFE;
  word_3AE6 &= 0x0001;
  word_3AE6 |= flags;
  cpu.ax = flags;
}

// 0x3C42
static void op_9A(void)
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;
  al = 0xFF;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  set_game_state(__func__, cpu.bx, al);
  uint8_t ah = (cpu.ax & 0xFF00) >> 8;
  if (byte_3AE1 != ah) {
    set_game_state(__func__, cpu.bx + 1, al);
  }
}

// 0x4A79
// also 0x4A7D (taking 3AE2)
static void sub_4A79(uint8_t al)
{
  cpu.ax = al;

  // 0x4A80
  cpu.di = cpu.ax;
  cpu.bx = cpu.ax;

  al = *cpu.pc++;
  cpu.bx = cpu.bx >> 3;
  cpu.bx += al;
  cpu.di &= 7;
  al = data_4A99[cpu.di]; // bit flags
  cpu.ax = al;
}

// 0x416B
static void op_9B(void)
{
  uint8_t al = *cpu.pc++;
  sub_4A79(al);
  set_game_state(__func__, cpu.bx, game_state.unknown[cpu.bx] | (cpu.ax & 0xFF));
}

// 0x4181
static void op_9D(void)
{
  uint8_t al = *cpu.pc++;
  sub_4A79(al);

  // test [bx+3860], al
  cpu.cf = 0;
  cpu.zf = (game_state.unknown[cpu.bx] & cpu.ax) == 0 ? 1 : 0;
  set_flags();
}

// Extract string from stream, call func for each character
// 0x1C79
uint16_t extract_string(const unsigned char *src_ptr, uint16_t offset, void (*func)(unsigned char))
{
  uint8_t ret, bl;

  bit_extractor_info.num_bits = 0;
  bit_extractor_info.bit_buffer = 0;
  bit_extractor_info.data = src_ptr;
  bit_extractor_info.offset = offset;
  bit_extractor_info.upper_case = 0;

  // 0x1C8B
  while (1) {
    ret = extract_letter(&bit_extractor_info); // check for 0
    if (ret == 0) {
      // 1CE6
      return bit_extractor_info.offset;
    }
    if ((game_state.unknown[8] & 0x80) == 0)
    {
      ret |= 0x80;
      game_state.unknown[8] = ret;
      ret &= 0x7F;
    }

    // Check for escape codes.
    // 1C9E
    if (ret == 0xAF || ret == 0xDC) {
      do {
        if (ret == 0xAF) {
          // 0x1CAB
          bl = 0x80;
        } else if (ret == 0xDC) {
          // 0x1CAF
          bl = 0;
        }
        // 0x1CB1
        if (ret == 0xAF || ret == 0xDC) {
          byte_1CE1 = ret;
          if (game_state.unknown[9] != 0) {
            // 0x1CBB
            bl = bl ^ 0x80;
          }
          byte_1CE2 = bl;
        }
        // 0x1CC2
        ret = extract_letter(&bit_extractor_info); // check for 0
        if (ret == 0) {
          // 1CE6
          return bit_extractor_info.offset;
        }
        if (ret == byte_1CE1) {
          break;
        }
        if (ret == 0xDC || ret == 0xAF) {
          continue;
        }
        if (byte_1CE2 != 0) {
          func(ret);
        }
      } while (1);
    } else {
      func(ret);
    }
  }
}

// 0x3191
// Append's a byte to string buffer. This may break a long string up into
// multiple lines.
static void append_string(unsigned char byte)
{
  uint8_t al;
  uint16_t ax, bx, len, dx;

  bx = ui_string.len;
  ui_string.bytes[bx] = byte;
  ui_string.len++;

  printf("%s: 0x%02X %c\n", __func__, byte, byte & 0x7F);
  if (byte == 0x8D) { // new line.
    printf("data len: %d\n", ui_string.len);
    printf("2697: 0x%04x\n", draw_rect.x);
    printf("2697: 0x%04x\n", draw_rect.y);
    printf("32BF: 0x%04x\n", draw_point.x);
    printf("32BF: 0x%04x\n", draw_point.y);
    ui_draw_string();
    return;
  }

  ax = bx;

  // Validate that string doesn't run past rectangle.
  ax += ui_get_byte_3236();
  if (ax < draw_rect.w)
    return;

  // String does run past rectangle, so break apart at space.
  // 0x31AE
  ui_string.len--;
  if (ui_string.len != 0) {
    len = bx;

    // 0x31B6
    while (len > 0) {
      al = ui_string.bytes[len];
      // search for previous space character.
      if (al == 0xA0) {
        bx = len;
        printf("%s: 0x31D2 %02d\n", __func__, len);
        if (len != 0) {
          // mov si, 320E
          uint16_t si = 0;

          // redraw only up to space (line break)
          while (len > 0) {
            // lodsb  al = ds:si [si++]
            al = ui_string.bytes[si++];
            ui_draw_chr_piece(al);
            len--;
          }
        }
        // 31E2
        ui_string.len++;
        len = 0;

        // 0x31E8
        bx++;
        while (bx < ui_string.len) {

          // 0x31EF
          // move string up.
          al = ui_string.bytes[bx];
          ui_string.bytes[len] = al;
          len++;
          bx++;
        }
        // 0x31FA
        dx = len;
        break;
      }
      len--;
    }
  }
  else {
    // 31C1 - length is 0.
    ui_draw_string();
    ui_string.bytes[0] = ui_string.bytes[bx];
    dx = 1;
  }
  // 0x31FC
  ui_string.len = dx;
  al = 0x8D;
  ui_draw_chr_piece(al);
  ui_set_byte_3236(draw_point.x);
}

static void set_sb_handler_ui_draw_chr()
{
  string_byte_handler_func = ui_draw_chr_piece;
}

static void set_sb_handler_append_string()
{
  string_byte_handler_func = append_string;
}

// 0x27E3
static void set_ui_header(unsigned char *base_ptr, uint16_t offset)
{
  // Indicate that we are interested in setting the header.
  string_byte_handler_func = ui_header_set_byte;
  ui_string.len = 0;
  ui_header_reset();

  // Extract string (this will call string_byte_handler_func, a.k.a. ui_header_set_byte)
  cpu.bx = extract_string(base_ptr, offset, handle_byte_callback);

  // Reset function ptr.
  set_sb_handler_append_string();

  // Flush to screen
  sub_280E();
}

static void sub_280E()
{
  // check length against previous length (max length?)
  // XXX: Unknown.
  ui_header_draw();
  vga_update();
}

// 0x3150
static void handle_byte_callback(unsigned char byte)
{
  string_byte_handler_func(byte);
}

// 0x482D
static void read_header_bytes(void)
{
  set_ui_header(cpu.base_pc, cpu.pc - cpu.base_pc);
  cpu.pc = cpu.base_pc + cpu.bx;
}

// 0x3AA0
static void run_script(uint8_t script_index, uint16_t src_offset)
{
  int done = 0;
  uint8_t prev_op = 0;
  uint8_t op_code = 0;

  uint8_t cl = word_3AE8;
  cpu.cx = (cpu.cx & 0xFF00) | cl;

  push_word(cpu.cx);
  push_word(word_3ADB);
  push_word(saved_stack);
  saved_stack = cpu.sp;

  word_3AE8 = script_index;
  word_3AEA = script_index;
  populate_3ADD_and_3ADF();

  cpu.pc = running_script->bytes + src_offset;
  cpu.base_pc = running_script->bytes;

  uint16_t offset;

  while (!done) {
    // For debugging:
    offset = (uint16_t)(cpu.pc - cpu.base_pc);

    prev_op = op_code;
    // 0x3ACF
    // es lodsb
    op_code = *cpu.pc++;
    // xor ah, ah
    cpu.ax = op_code;
    cpu.bx = cpu.ax;

    // TEMPORARY: Dump script and offset
    //printf("%02d: 0x%04X ", running_script->tag, offset);

    void (*callfunc)(void) = targets[op_code].func;
    if (callfunc != NULL) {
      callfunc();
      if (op_code == 0x5A)
        done = 1;

    } else {
      printf("OpenDW has reached an unhandled op code and will terminate.\n");
      printf("  Script: %d - 0x%04X\n", running_script->tag, offset);
      printf("  Opcode: 0x%02X (Addr: %s), Previous op: 0x%02X\n", op_code,
          targets[op_code].src_offset, prev_op);
      exit(1);
      done = 1;
    }
  }
}

void run_engine()
{
  // Initialize timers.
  timers.timer0 = 1;
  timers.timer1 = 1;
  timers.timer2 = 1;
  timers.timer3 = 1;
  timers.timer4 = 1;
  timers.timer5 = 0;

  game_state.unknown[8] = 0xFF;
  memset(&cpu, 0, sizeof(struct virtual_cpu));
  cpu.sp = STACK_SIZE; // stack grows downward...

  ui_set_background(0x0000); // Not correct.

  // load common "ESC" string table (compressed bits).
  escape_string_table = com_extract(0x2A68, 0x39);

  // load unknown data from COM file.
  data_D760 = com_extract(0xD760, 0x700);
  data_1E21 = com_extract(0x1E21, 0xEF);

  // 0x1A6
  // Loads into 0x1887:0000
  const struct resource *code_res = resource_load(RESOURCE_SCRIPT);
  if (code_res == NULL)
  {
    printf("Failed to load unknown resource\n");
    return;
  }
  printf("Resource bytes: %zu\n", code_res->len);
  dump_hex(code_res->bytes, 0x80);

  // 0x3AA0
  run_script(code_res->index, 0);

  free(data_D760);
}

static int sub_1C57()
{
  unsigned char *c960 = get_player_data_base();

  cpu.ax = c960[player_base_offset - 0xC960 + cpu.bx];
  cpu.ax += (c960[player_base_offset - 0xC960 + 1 + cpu.bx]) << 8;

  word_11C0 = cpu.ax;
  return cpu.ax;
}

static void sub_11A0(int set_11C4)
{
  uint32_t result;

  word_11C4 = set_11C4;

  // 11A6
  cpu.ax = word_11C2;
  result = cpu.ax * word_11C0;
  word_11C6 = result & 0xFFFF;
  word_11C8 = (result & 0xFFFF0000) >> 16;
  // What is the point of this? multiplying by 0 ??
  cpu.ax = word_11C4;
  result = cpu.ax * word_11C0;
  word_11C8 += result & 0xFFFF;
}

static void sub_11CE()
{
  int old_carry = 0;
  int carry = 0;

  word_11CA = 0;
  word_11CC = 0;
  cpu.cx = 0x20; // 32 times.

  // 11DD
  for (uint16_t i = 0; i < cpu.cx; i++) {
    carry = word_11C6 & 0x8000 ? 1 : 0;
    word_11C6 = word_11C6 << 1;
    old_carry = carry;

    // rcl word_11C8, 1
    carry = word_11C8 & 0x8000 ? 1 : 0;
    word_11C8 = (word_11C8 << 1) + old_carry;
    old_carry = carry;

    carry = word_11CA & 0x8000 ? 1 : 0;
    word_11CA = (word_11CA << 1) + old_carry;
    old_carry = carry;

    carry = word_11CC & 0x8000 ? 1 : 0;
    word_11CC = (word_11CC << 1) + old_carry;
    old_carry = carry;

    printf("%s: 0x%04X 0x%04X 0x%04X 0x%04X\n", __func__, word_11C6, word_11C8, word_11CA, word_11CC);

    cpu.ax = word_11CA;
    uint16_t old16 = cpu.ax;
    cpu.ax -= word_11C0;
    if (cpu.ax > old16) {
      carry = 1;
    } else {
      carry = 0;
    }
    cpu.bx = word_11CC;
    old16 = cpu.bx;
    cpu.bx -= carry;

    if (cpu.bx > old16) {
      carry = 1;
    } else {
      carry = 0;
    }

    if (carry != 1) {
      word_11CA = cpu.ax;
      word_11CC = cpu.bx;
      word_11C6++;
    }
  }
}

static void sub_1C49(uint16_t fill_color)
{
  g_linenum++;
  ui_draw_solid_color(fill_color, word_36C0, word_36C2, g_linenum);
  g_linenum--;
  ui_draw_solid_color(fill_color, word_36C0, word_36C2, g_linenum);
}

static void sub_1BF8(uint8_t color, uint8_t y_adjust)
{
  uint16_t fill_color;

  fill_color = color;
  cpu.ax = draw_point.y + y_adjust;
  g_linenum = cpu.ax; // line number
  if (sub_1C57() != 0) {
    cpu.bx += 2;
    push_word(cpu.bx);
    word_11C2 = 0x17;
    sub_11A0(0);
    cpu.bx = pop_word();
    if (sub_1C57() != 0) {
      sub_11CE();
      cpu.ax = word_11C6;
      cpu.ax++;
    }
  }
  cpu.ax += 0x36;
  word_36C2 = cpu.ax;
  word_36C0 = 0x36;
  sub_1C49(fill_color);
  cpu.ax = word_36C2;
  if (cpu.ax == 0x4E) {
    return;
  }

  // 1C3A
  word_36C0 = cpu.ax;
  word_36C2 = 0x4E;
  fill_color = byte_1BE5; // color.
  sub_1C49(fill_color);
}

