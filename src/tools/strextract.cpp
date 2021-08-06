//
// Copyright (c) 2021 Devin Smith <devin@devinsmith.net>
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

// Dragon Wars script disassembler.

#include <cstdio>
#include <stdint.h>

#include <engine.h>

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
  unsigned char data1[] = { 0xfb, 0x9f, 0xab, 0xed, 0x06, 0x6f, 0x09, 0xbe, 0x12, 0xa6, 0x00 };
  unsigned char data2[] = { 0xfb, 0x9f, 0xab, 0xed, 0x06, 0x6f, 0x09, 0x1e, 0xe9, 0xa9, 0xd4, 0x30, 0x00 };
  unsigned char data3[] = { 0xf4, 0x22, 0x69, 0x48, 0x3f, 0x73, 0xf5, 0x7d, 0xa0, 0x00 };
  unsigned char data4[] = { 0xfb, 0x9f, 0xab, 0xed, 0x06, 0x6f, 0x0a, 0x1e, 0x11, 0x88, 0x8b, 0x00 };

  unsigned char world0[] = { 0xf4, 0x29, 0x14, 0x0a, 0x6f, 0x8d, 0xc0 };

  extract_test(data1);
  extract_test(data2);
  extract_test(data3);
  extract_test(data4);
  extract_test(world0);
  return 0;
}
