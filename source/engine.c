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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "engine.h"
#include "resource.h"
#include "ui.h"
#include "utils.h"
#include "vga.h"

/* Represents the engine that Dragon wars runs.
 *
 * It appears that Dragon Wars executes a script where each op code
 * does some action.
 *
 * Lots of global variables here until we can figure out how they are used. */

uint8_t byte_1CE1 = 0;
uint8_t byte_1CE2 = 0;

// 0x1CE3
// Represents number of bits that are left over to read from byte_1CE5.
uint8_t num_bits = 0;

uint8_t byte_1CE4 = 0;
uint8_t byte_1CE5 = 0;
uint8_t byte_3AE1 = 0;
uint16_t word_3AE2 = 0;
uint16_t word_3AE4 = 0;
uint16_t word_3AE6 = 0;
uint16_t word_3AE8 = 0;
uint16_t word_3AEA = 0;

uint16_t word_3ADB = 0;

/* 0x3ADD */
const struct resource *running_script = NULL;
const struct resource *word_3ADF = NULL;

uint16_t word_42D6 = 0;
unsigned char *data_D760 = NULL;

// The function signature for this function pointer is not entirely correct
// but we'll figure it out as we decode more of DW.
void (*word_3163)(unsigned char byte);

struct game_state {
  unsigned char unknown[256];
};

// 0x3860
static struct game_state game_state = {0};

// virtual CPU
struct virtual_cpu {
  // registers
  uint16_t ax;
  uint16_t bx;
  uint16_t cx;

  uint16_t di;

  // stack
  uint16_t sp[16]; // stacks;
  int stack_index;

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

struct virtual_cpu cpu;

static void run_script(uint8_t script_index, uint16_t src_offset);
static uint8_t sub_1CF8();
static uint8_t sub_1D8A();
static void sub_3150(unsigned char byte);
static void sub_316C();
static void sub_3191(unsigned char byte);
static void sub_280E();

// Decoded opcode calls, foward definition.
static void op_00();
static void op_01();
static void op_03();
static void op_04();
static void op_05();
static void op_06();
static void op_09();
static void op_0A();
static void op_0F();
static void op_11();
static void op_12();
static void op_13();
static void op_17();
static void op_1A();
static void op_1C();
static void op_1D();
static void op_23();
static void op_26();
static void op_3E();
static void op_40();
static void op_41();
static void op_42();
static void op_44();
static void op_45();
static void loop(); // 49
static void op_4A();
static void op_4B();
static void op_53();
static void op_54();
static void op_56();
static void op_5C();
static void op_66();
static void read_header_bytes(void); // 7B
static void op_84();
static void op_85();
static void op_86();
static void op_93();
static void op_99();
static void op_9A();

struct op_call_table {
  void (*func)();
  const char *src_offset;
};

struct op_call_table targets[] = {
  { op_00, "0x3B18" },
  { op_01, "0x3B0E" },
  { NULL, "0x3B1F" },
  { op_03, "0x3B2F" },
  { op_04, "0x3B2A" },
  { op_05, "0x3B3D" },
  { op_06, "0x3B4A" },
  { NULL, "0x3B52" },
  { NULL, "0x3B59" },
  { op_09, "0x3B67" },
  { op_0A, "0x3B7A" },
  { NULL, "0x3B8C" },
  { NULL, "0x3BA2" },
  { NULL, "0x3BB7" },
  { NULL, "0x3BD0" },
  { op_0F, "0x3BED" },
  { NULL, "0x3C10" },
  { op_11, "0x3C2D" },
  { op_12, "0x3C59" },
  { op_13, "0x3C72" },
  { NULL, "0x3C8F" },
  { NULL, "0x3CAB" },
  { NULL, "0x3CCB" },
  { op_17, "0x3CEF" },
  { NULL, "0x3D19" },
  { NULL, "0x3D3D" },
  { op_1A, "0x3D5A" },
  { NULL, "0x3D73" },
  { op_1C, "0x3D92" },
  { op_1D, "0x4ACC" },
  { NULL, "0x01B2" },
  { NULL, "0x4AF6" },
  { NULL, "0x0000" },
  { NULL, "0x3DAE" },
  { NULL, "0x3DB7" },
  { op_23, "0x3DC0" },
  { NULL, "0x3DD7" },
  { NULL, "0x3DE5" },
  { op_26, "0x3DEC" },
  { NULL, "0x3E06" },
  { NULL, "0x3E14" },
  { NULL, "0x3E1B" },
  { NULL, "0x3E36" },
  { NULL, "0x3E45" },
  { NULL, "0x3E4C" },
  { NULL, "0x3E67" },
  { NULL, "0x3E6E" },
  { NULL, "0x3E75" },
  { NULL, "0x3E9D" },
  { NULL, "0x3EC1" },
  { NULL, "0x3EEB" },
  { NULL, "0x3F11" },
  { NULL, "0x3F4D" },
  { NULL, "0x3F66" },
  { NULL, "0x3F8C" },
  { NULL, "0x3FAD" },
  { NULL, "0x3FBC" },
  { NULL, "0x3FD4" },
  { NULL, "0x3FEA" },
  { NULL, "0x4002" },
  { NULL, "0x4018" },
  { NULL, "0x4030" },
  { op_3E, "0x4051" },
  { NULL, "0x4067" },
  { op_40, "0x4074" },
  { op_41, "0x407C" },
  { op_42, "0x4085" },
  { NULL, "0x408E" },
  { op_44, "0x4099" },
  { op_45, "0x40A3" },
  { NULL, "0x40AF" },
  { NULL, "0x40B8" },
  { NULL, "0x40ED" },
  { loop, "0x4106" },
  { op_4A, "0x4113" },
  { op_4B, "0x4122" },
  { NULL, "0x412A" },
  { NULL, "0x4132" },
  { NULL, "0x414B" },
  { NULL, "0x4155" },
  { NULL, "0x4161" },
  { NULL, "0x418B" },
  { NULL, "0x41B9" },
  { op_53, "0x41C0" },
  { op_54, "0x41E1" },
  { NULL, "0x41E5" },
  { op_56, "0x41FD" },
  { NULL, "0x4215" },
  { NULL, "0x4239" },
  { NULL, "0x41C8" },
  { NULL, "0x3AEE" },
  { NULL, "0x427A" },
  { op_5C, "0x4295" },
  { NULL, "0x42D8" },
  { NULL, "0x4322" },
  { NULL, "0x4372" },
  { NULL, "0x438B" },
  { NULL, "0x43A6" },
  { NULL, "0x43BF" },
  { NULL, "0x43F7" },
  { NULL, "0x446E" },
  { NULL, "0x44B8" },
  { op_66, "0x40C1" },
  { NULL, "0x44CB" },
  { NULL, "0x450A" },
  { NULL, "0x453F" },
  { NULL, "0x4573" },
  { NULL, "0x45A1" },
  { NULL, "0x45A8" },
  { NULL, "0x45F0" },
  { NULL, "0x45FA" },
  { NULL, "0x4607" },
  { NULL, "0x4632" },
  { NULL, "0x465B" },
  { NULL, "0x46B6" },
  { NULL, "0x47B7" },
  { NULL, "0x47C0" },
  { NULL, "0x47D1" },
  { NULL, "0x47D9" },
  { NULL, "0x47E3" },
  { NULL, "0x47EC" },
  { NULL, "0x47FA" },
  { NULL, "0x4801" },
  { read_header_bytes, "0x482D" },
  { NULL, "0x4817" },
  { NULL, "0x483B" },
  { NULL, "0x4845" },
  { NULL, "0x486D" },
  { NULL, "0x487F" },
  { NULL, "0x48C5" },
  { NULL, "0x48D2" },
  { NULL, "0x48EE" },
  { op_84, "0x4907" },
  { op_85, "0x4920" },
  { op_86, "0x493E" },
  { NULL, "0x4955" },
  { NULL, "0x496D" },
  { NULL, "0x4977" },
  { NULL, "0x498E" },
  { NULL, "0x499B" },
  { NULL, "0x49A5" },
  { NULL, "0x49D3" },
  { NULL, "0x0000" },
  { NULL, "0x49DD" },
  { NULL, "0x49E7" },
  { NULL, "0x49F3" },
  { NULL, "0x49FD" },
  { op_93, "0x4A67" },
  { NULL, "0x4A6D" },
  { NULL, "0x4894" },
  { NULL, "0x48B5" },
  { NULL, "0x42FB" },
  { NULL, "0x4348" },
  { op_99, "0x40E7" },
  { op_9A, "0x3C42" },
  { NULL, "0x416B" },
  { NULL, "0x4175" },
  { NULL, "0x4181" },
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

static void push_word(uint16_t val)
{
  if (cpu.stack_index == 0) {
    cpu.stack_index = 16;
  }

  cpu.stack_index--;
  cpu.sp[cpu.stack_index] = val;
}

static void push_byte(uint8_t val)
{
  if (cpu.stack_index == 0) {
    cpu.stack_index = 16;
  }

  // we need to push a byte onto the stack.
  cpu.stack_index--;
  cpu.sp[cpu.stack_index] = val;
}

static uint8_t pop_byte()
{
  uint8_t val = cpu.sp[cpu.stack_index];
  cpu.stack_index++;
  if (cpu.stack_index >= 16)
    cpu.stack_index = 0;

  return val;
}

static uint16_t pop_word()
{
  uint16_t val = cpu.sp[cpu.stack_index];
  cpu.stack_index++;
  if (cpu.stack_index >= 16)
    cpu.stack_index = 0;

  return val;
}

// 0x3B18
static void op_00()
{
  byte_3AE1 = 0xFF;
}

// 0x3B0E
static void op_01()
{
  uint8_t ah = (cpu.ax & 0xFF00) >> 8;

  // moves AH into two variables.
  word_3AE2 = (ah << 8) | (word_3AE2 & 0xFF); // lower portion of word_3AE2 takes ah.
  byte_3AE1 = ah;
}

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
static void op_06()
{
  uint8_t al = *cpu.pc++;
  word_3AE4 = al;
}

// 0x3B67
static void op_09(void)
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
static void op_0A(void)
{
  uint8_t al, ah;
  static int runs = 1;

  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;

  // mov ax, [bx + game_state]
  al = game_state.unknown[cpu.bx];
  ah = game_state.unknown[cpu.bx+1];
  ah = ah & byte_3AE1;
  cpu.ax = (ah << 8) | al;
  word_3AE2 = cpu.ax;
  printf("OP_0A - AX: 0x%04X (run num: %d)\n", cpu.ax, runs);
  runs++;
}

// 0x3BED
static void op_0F(void)
{
  uint8_t ah, al;
  al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;
  // load di properly...
  printf("OP_0F: BX: 0x%04X\n", cpu.bx);
  cpu.di = game_state.unknown[cpu.bx];
  cpu.di += (game_state.unknown[cpu.bx + 1] << 8);
  uint8_t bl = game_state.unknown[cpu.bx + 2];
  cpu.bx = (cpu.bx & 0xFF00) | bl;

  const struct resource *r = resource_get_by_index(cpu.bx);
  //dump_hex(r->bytes, 0x10);

  cpu.di += word_3AE4;

  // little endian load
  // mov ax, es:[di]
  al = r->bytes[cpu.di];
  ah = r->bytes[cpu.di + 1];
  cpu.ax = (ah << 8) | al;
  ah = ah & byte_3AE1;
  cpu.ax = ah << 8 | al;
  word_3AE2 = (ah << 8) | al;
}

// 0x3C2D
static void op_11(void)
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;
  uint8_t ah = (cpu.ax & 0xFF00) >> 8;
  printf("op_11: 0x%04X ah: 0x%02X\n", cpu.bx, ah);
  game_state.unknown[cpu.bx] = ah;
  if (byte_3AE1 != ah) {
    game_state.unknown[cpu.bx + 1] = ah;
  }
}

// 0x3C59
static void op_12(void)
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;
  cpu.cx = word_3AE2;
  game_state.unknown[cpu.bx] = (cpu.cx & 0xFF);
  if (byte_3AE1 != ((cpu.ax & 0xFF00) >> 8)) {
    game_state.unknown[cpu.bx+1] = ((cpu.cx & 0xFF00) >> 8);
  }
}

// 0x3C72
static void op_13(void)
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;
  cpu.cx = word_3AE2;
  cpu.bx += word_3AE4;
  printf("op_13: 0x%04X\n", cpu.bx);
  game_state.unknown[cpu.bx] = (cpu.cx & 0x00FF);
  if (byte_3AE1 != ((cpu.ax & 0xFF00) >> 8)) {
    game_state.unknown[cpu.bx + 1] = ((cpu.cx & 0xFF00) >> 8);
  }
}

// 0x3CEF
static void op_17(void)
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;
  cpu.di = game_state.unknown[cpu.bx];
  cpu.di += (game_state.unknown[cpu.bx + 1] << 8);
  uint8_t bl = game_state.unknown[cpu.bx + 2];
  cpu.bx = (cpu.bx & 0xFF00) | bl;
  printf("op17  bl: 0x%02X di: 0x%04X\n", bl, cpu.di);
  const struct resource *r = resource_get_by_index(bl);
  //dump_hex(r->bytes, 0x10);
  cpu.di += word_3AE4;
  cpu.cx = word_3AE2;
  printf("  op17: setting byte 0x%04X\n", cpu.di);
  r->bytes[cpu.di] = (cpu.cx & 0x00FF);
  if (byte_3AE1 != ((cpu.ax & 0xFF00) >> 8)) {
    r->bytes[cpu.di + 1] = ((cpu.cx & 0xFF00) >> 8);
  }
}

// 0x3D5A
static void op_1A(void)
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.di = cpu.ax;
  al = *cpu.pc++;
  game_state.unknown[cpu.di] = al;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  if (byte_3AE1 != ((cpu.ax & 0xFF00) >> 8)) {
    al = *cpu.pc++;
    game_state.unknown[cpu.di + 1] = al;
    cpu.ax = (cpu.ax & 0xFF00) | al;
  }
}

// 0x3D92
static void op_1C()
{
  uint16_t save_ah = (cpu.ax & 0xFF00) >> 8;

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
  memcpy(src + src_offset, dest + dest_offset, 0x700);
}

// 0x3DC0
static void op_23(void)
{
  // INCREMENT [memory]

  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.di = cpu.ax;
  game_state.unknown[cpu.di]++;
  if (game_state.unknown[cpu.di] == 0) {
    if (byte_3AE1 != ((cpu.ax & 0xFF00) >> 8)) {
      game_state.unknown[cpu.di+1]++;
    }
  }
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
  game_state.unknown[cpu.di] = cl;
  if (byte_3AE1 != ((cpu.ax & 0xFF00) >> 8)) {
    game_state.unknown[cpu.di + 1] = ch;
  }
}

// 0x4051
static void op_3E(void)
{
  uint8_t ah, al;
  int cf = 0;
  int zf = 0;
  cpu.bx = word_3AE2;
  ah = ((cpu.ax & 0xFF00) >> 8);
  if (byte_3AE1 != ah) {
    al = *cpu.pc++;
    ah = *cpu.pc++;
    cpu.ax = (ah << 8) | al;
    cf = (cpu.bx - cpu.ax) < 0;
    zf = (cpu.bx - cpu.ax) == 0 ? 1 : 0;
  } else {
    uint8_t bl = cpu.bx & 0x00FF;
    al = *cpu.pc++;
    cpu.ax = (cpu.ax & 0xFF00) | al;
    cf = (bl - al) < 0;
    zf = (bl - al) == 0 ? 1 : 0;
  }

  // loc_4042
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

// 0x4099
// op_44
static void op_44(void)
{
  if ((word_3AE6 & ZERO_FLAG_MASK) == 0) {
    cpu.pc++;
    cpu.pc++;
    return;
  }
  uint16_t new_address = *cpu.pc++;
  new_address += *cpu.pc++ << 8;
  cpu.ax = new_address;
  printf("(op44)    New address: 0x%04x\n", new_address);
  cpu.pc = cpu.base_pc + new_address;
}

// 0x40A3
static void op_45(void)
{
  if ((word_3AE6 & ZERO_FLAG_MASK) != 0) {
    cpu.pc++;
    cpu.pc++;
    return;
  }
  uint16_t new_address = *cpu.pc++;
  new_address += *cpu.pc++ << 8;
  cpu.ax = new_address;
  printf("(op45)    New address: 0x%04x\n", new_address);
  cpu.pc = cpu.base_pc + new_address;
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
static void op_4B(void)
{
  word_3AE6 |= 0x0001;
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
  printf("SI: %04X\n", si);
  cpu.pc = cpu.base_pc + si;
}

// 0x41FD
static void op_56(void)
{
  cpu.cx = word_3AE2;

  uint8_t ah = (cpu.ax & 0xFF00) >> 8;
  if (byte_3AE1 != ah) {
    printf("op_56 not done, exiting here\n");
    cpu.sp[cpu.stack_index++] = cpu.cx;
    exit(1);
  } else {
    // store cl into stack.
    push_byte(cpu.cx & 0xFF);
  }
}

// 0x4295
static void op_5C(void)
{
  op_01();
  cpu.ax = *cpu.pc++;
  cpu.ax += ((*cpu.pc++) << 8);

  word_42D6 = cpu.ax;
  word_3ADB = cpu.pc - cpu.base_pc; // is this correct?

  if (game_state.unknown[0x1F] == 0) {
    printf("Breakpoint 0x42D3\n");
    exit(1);
  } else {
    uint8_t al = game_state.unknown[6];
    cpu.ax = (cpu.ax & 0xFF00) | al;
    push_word(cpu.ax);
    game_state.unknown[6] = 0;
    cpu.bx = word_42D6;
    al = word_3AE8;
    printf("Breakpoint 0x42BF\n");
    run_script(al, word_42D6);
    exit(1);
  }
}

// 0x40C1
static void op_66(void)
{
  uint8_t al = *cpu.pc++;
  cpu.ax = (cpu.ax & 0xFF00) | al;
  cpu.bx = cpu.ax;

  int zf = 0;
  int cf = 0;

  cpu.cx = game_state.unknown[cpu.bx];
  cpu.cx += (game_state.unknown[cpu.bx + 1] << 8);
  if (byte_3AE1 == (cpu.ax >> 8)) {
    uint8_t cl = cpu.cx & 0x00FF;
    if ((cl & cl) == 0) {
      zf = 1;
    }
  } else {
    if ((cpu.cx & cpu.cx) == 0) {
      zf = 1;
    }
  }

  uint16_t flags = 0;
  flags |= zf << 6;
  flags |= 1 << 1; // Always 1, reserved.
  flags |= cf << 0;
  flags &= 0xFFFE;
  word_3AE6 &= 0x0001;
  word_3AE6 |= flags;
  cpu.ax = flags;
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
static void op_86(void)
{
  const struct resource *r = resource_load(word_3AE2);
  cpu.ax = r->index;
  uint8_t ah = (cpu.ax & 0xFF00) >> 8;
  word_3AE2 = (ah & byte_3AE1) | (cpu.ax & 0x00FF);
  //dump_hex(r->bytes, 0x80);
}

// 0x4A67
static void op_93(void)
{
  uint8_t byte_3AE4 = (word_3AE4 & 0x00FF);
  uint8_t al = byte_3AE4;
  cpu.ax = (cpu.ax & 0xFF00) | al;

  push_byte(al);
}

// 0x40E7
static void op_99(void)
{
  cpu.cx = word_3AE2;

  // XXX: Figure out what test this is really trying to do.
  // Zero flag, carry flag, sign flag?

  int zf = 0;
  int cf = 0;
  // 40C9
  if (byte_3AE1 == (cpu.ax >> 8)) {
    uint8_t cl = cpu.cx & 0x00FF;
    if ((cl & cl) == 0) {
      zf = 1;
    }
  } else {
    if ((cpu.cx & cpu.cx) == 0) {
      zf = 1;
    }
  }

  uint16_t flags = 0;
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
  game_state.unknown[cpu.bx] = al;
  uint8_t ah = (cpu.ax & 0xFF00) >> 8;
  if (byte_3AE1 != ah) {
    game_state.unknown[cpu.bx + 1] = al;
  }
}

static void sub_1C79(void)
{
  num_bits = 0;

  byte_1CE4 = 0;
  while (1) {
    uint8_t ret = sub_1CF8(); // check for 0
    if (ret == 0) {
      // 1CE6
      return;
    }
    if ((game_state.unknown[8] & 0x80) == 0)
    {
      ret |= 0x80;
      game_state.unknown[8] = ret;
      ret &= 0x7F;
    }
    // 1C9E
    if (ret == 0xAF) {
      // 0x1CAB
      printf("Unhandled engine code: 0x1CAB\n");
      return;
    }
    if (ret == 0xDC) {
      // 0x1CAF
      printf("Unhandled engine code: 0x1CAF\n");
      return;
    }
    sub_3150(ret);
  }
}

// 0x1D2A - 0x1D85
// Some type of character alphabet?
unsigned char alphabet[] = {
  0xa0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xeb, 0xec,
  0xed, 0xee, 0xef, 0xf0, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf9, 0xae,
  0xa2, 0xa7, 0xac, 0xa1, 0x8d, 0xea, 0xf1, 0xf8, 0xfa, 0xb0, 0xb1, 0xb2,
  0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0x30, 0x31, 0x32, 0x33, 0x34,
  0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
  0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53,
  0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0xa8, 0xa9, 0xaf, 0xdc, 0xa3,
  0xaa, 0xbf, 0xbc, 0xbe, 0xba, 0xbb, 0xad, 0xa5
};

static uint8_t sub_1CF8()
{
  while (1) {
    uint8_t ret = sub_1D8A();
    if (ret == 0) {
      return 0;
    } else if (ret < 0x1E) {
      // 0x1D0A
      // offset
      uint8_t al = alphabet[ret - 1];
      byte_1CE4 = byte_1CE4 >> 1;
      if (byte_1CE4 >= 0x40 && al >= 0xE1 && al <= 0xFA) {
        al = al & 0xDF;
      }
      // test al, al
      return al;
    } else if (ret == 0x1E) {
      // stc
      // rcr byte [byte_1CE4], 1
      byte_1CE4 = byte_1CE4 >> 1;
      byte_1CE4 += 0x80;
    }
  }
}

// Extract 5 bits out of each byte.
static uint8_t sub_1D8A()
{
  int counter = 5;
  int al = 0;
  int dl = num_bits;
  while (counter > 0) {
    dl--;
    if (dl < 0) {
      dl = *cpu.pc;
      printf("DL=0x%02x\n", dl);
      byte_1CE5 = dl;
      dl = 7;
      cpu.pc++;
    }
    // 0x1D96
    uint8_t tmp = byte_1CE5;
    byte_1CE5 = byte_1CE5 << 1;

    // rcl al, 1
    int carry = 0;
    if (tmp > byte_1CE5) {
      carry = 1;
    }
    al = al << 1;
    al += carry;
    counter--;
  }
  num_bits = dl;
  return al;
}

static void sub_3191(unsigned char byte)
{
}

static void sub_316C()
{
  word_3163 = sub_3191;
}

static void sub_27E3()
{
  word_3163 = ui_header_set_byte;
  ui_header_reset();
  sub_1C79();
  sub_316C();
  sub_280E();
}

static void sub_280E()
{
  // check length against previous length (max length?)
  // XXX: Unknown.
  ui_header_draw();
  vga->update();
}

// 0x3150
static void sub_3150(unsigned char byte)
{
  word_3163(byte);
}

// 0x482D
static void read_header_bytes(void)
{
  sub_27E3();
}

static void run_script(uint8_t script_index, uint16_t src_offset)
{
  int done = 0;
  uint8_t prev_op = 0;
  uint8_t op_code = 0;

  word_3AE8 = script_index;
  word_3AEA = script_index;
  populate_3ADD_and_3ADF();

  cpu.pc = running_script->bytes + src_offset;
  cpu.base_pc = cpu.pc;

  while (!done) {
    prev_op = op_code;
    // es lodsb
    op_code = *cpu.pc++;
    // xor ah, ah
    cpu.ax = op_code;
    cpu.bx = cpu.ax;

    void (*callfunc)(void) = targets[op_code].func;
    if (callfunc != NULL) {
      callfunc();
    } else {
      printf("OpenDW has reached an unhandled op code and will terminate.\n");
      printf("  Opcode: 0x%02X (Addr: %s), Previous op: 0x%02X\n", op_code,
          targets[op_code].src_offset, prev_op);
      done = 1;
    }
  }
}

void run_engine()
{
  game_state.unknown[8] = 0xFF;
  memset(&cpu, 0, sizeof(struct virtual_cpu));
  cpu.stack_index = 16; // stack grows downward...

  // load unknown data from COM file.
  data_D760 = com_extract(0xD760, 0x700);

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

void set_game_state(int offset, unsigned char value)
{
  game_state.unknown[offset] = value;
}
