// Copyright (c) 2022 Devin Smith <devin@devinsmith.net>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

// Dragon Wars resource extractor.

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "../lib/bufio.h"
#include "engine.h"

unsigned char gamestate_b[256];
unsigned short gamestate_s[256];

unsigned short script_data[] = { 0x3D6, 0x412, 0x404, 0x48A };

unsigned char monster_data[256];
unsigned char monster_name[20];

struct buf_rdr *load_file()
{
  FILE *fp = fopen("res31.bin", "rb");
  if (fp == nullptr) {
    fprintf(stderr, "Failed to load res31.bin\n");
    return nullptr;
  }

  fseek(fp, 0, SEEK_END);
  auto len = ftell(fp);
  auto *buffer = new unsigned char[len];
  fseek(fp, 0, SEEK_SET);
  auto bytes_read = fread(buffer, 1, len, fp);
  if (bytes_read != len) {
    fprintf(stderr, "Failed to read res31.bin.\n");
    delete[] buffer;
    fclose(fp);
    return nullptr;
  }

  auto *buf_rdr = buf_rdr_init(buffer, len);

  fclose(fp);

  return buf_rdr;
}

void sub_6B5(uint8_t input)
{
  uint8_t word_3AE4 = input;
  uint16_t ax = 0x4C6;

  word_3AE4 = word_3AE4 >> 1;
  ax += word_3AE4;

  uint16_t offset = script_data[ax];

  gamestate_s[0x41] = offset;

  offset += 10; // 0x0A

  // Check script data at offset.
  // test it (





}

struct len_string {
  int len;
  char string[255];
};
len_string str;

static void pop_len_string(unsigned char ch)
{
  str.string[str.len] = ch & 0x7f;
  str.len++;
}

static int extract_test(const unsigned char *args)
{
  str.len = 0;
  str.string[0] = '\0';

  uint16_t count = extract_string(args, 0, pop_len_string);
  printf("Count: %d\n", count);

  printf("$(\"");
  for (int i = 0; i < str.len; i++) {
    char ch = str.string[i];
    if (ch == '\r') {
      printf("\\r");
    } else {
      printf("%c", ch);
    }
  }
  printf("\")\n");

  return count;
}


int main(int argc, char *argv[])
{
  auto *rdr = load_file();
  if (rdr == nullptr) {
    return -1;
  }

  // 0x4F6
  rdr->offset = 2;
  uint16_t initial_offset = buf_get16le(rdr);
  gamestate_s[0x58] = initial_offset;

  printf("Multiplier offset: 0x%04X\n", initial_offset);

  // Now in byte mode.
  rdr->offset = initial_offset;
  uint8_t multiplier = buf_get8(rdr);
  printf("Multiplier: 0x%02X\n", multiplier);

  // A 1x Multiplier will not raise high ticks greater than 16 bit
  // Meaning that any 16 bit number multiplied by 1 is still a 16 bit number
  // op_4d will then return 0.
  uint16_t mul_result = 0;

  // Offset here could be determined.
  rdr->offset = initial_offset + (mul_result) + 1; // 0x50B

  uint16_t next_offset = buf_get16le(rdr);
  printf("Next offset: 0x%04X\n", next_offset);

  rdr->offset = next_offset;
  uint8_t unknown1 = buf_get8(rdr);
  printf("Unknown1: 0x%02X\n", unknown1);

  // 0x516
  // New multipler?
  gamestate_b[0x28] = (unknown1 >> 6) + 1; // 0x516
  gamestate_b[0x5c] = (unknown1 & 0x3F);  // 0x521

  printf("\nBytes:\n");
  for (int i = 1; i < 0xD; i++) {
    uint8_t unknown2 = buf_get8(rdr);
    if (i == 1) {
      printf("  0x%02X <-- New multiplier?\n", unknown2);
    } else {
      printf("  0x%02X\n", unknown2);
    }
    gamestate_b[i + 0x28] = unknown2;
  }

  // 0x52E and 0x530
  gamestate_b[0x47] = 0;
  gamestate_b[0x48] = 0;

  gamestate_s[0x58] = 0;

  rdr->offset = 0;
  uint16_t first_offset = buf_get16le(rdr);
  gamestate_s[0x58] = first_offset;

  printf("First offset: 0x%04X\n", first_offset);

  // Byte mode, 0x53A
  uint8_t byte_3AE4 = gamestate_b[0x48];
  uint8_t byte_3AE2 = gamestate_b[0x29 + byte_3AE4];
  byte_3AE2 &= 0x1f;
  byte_3AE2--;
  printf("Byte 3AE2: 0x%02X\n", byte_3AE2);

  if (byte_3AE2 >= 0x80) {
    // determine new multiplier ?
    rdr->offset = first_offset;
    multiplier = buf_get8(rdr);

    printf("Multiplier: 0x%02X\n", multiplier);

    mul_result = 0x18; // Hardcoding for now, but it's the result of multiplier times game ticks
    // See op_04 for more information.
    byte_3AE2 = mul_result;
  }

  // 0x54A
  byte_3AE4 = byte_3AE2;
  byte_3AE4 = byte_3AE4 << 1;
  byte_3AE4++;

  printf("054C: word_3AE4 = 0x%02X\n", byte_3AE4);

  rdr->offset = first_offset + byte_3AE4;
  uint16_t monster_offset = buf_get16le(rdr);

  printf("monster_offset = 0x%04X\n", monster_offset);

  rdr->offset = monster_offset;

  sub_6B5(gamestate_b[0x47]);

  byte_3AE4 = 0;

  for (int i = 0; i < 21; i++) {
    uint8_t unknown2 = buf_get8(rdr);
    printf("  0x%02X\n", unknown2);
    monster_data[i] = unknown2;
  }

  //rdr->offset += 8;
  rdr->offset = monster_offset + 0x21;

  printf("Name bytes:\n");
  for (int i = 0; i < 18; i++) {
    uint8_t unknown2 = buf_get8(rdr);
    printf("  0x%02X\n", unknown2);
    monster_name[i] = unknown2;
  }

  extract_test(monster_name);


  delete[] rdr->data;
  buf_rdr_free(rdr);
  return 0;
}
