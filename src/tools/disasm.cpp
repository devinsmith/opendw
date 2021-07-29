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
#include <cstdlib>
#include <cstring>
#include <stdint.h>

#include <engine.h>

static int read_string_bytes(const unsigned char *args);
static int read_word(const unsigned char *args);

struct op_code {
  const char *name;
  int (*func)(const unsigned char *args);
  int arg_count;
};

op_code op_codes[] = {
  { ".word", nullptr, 0 }, // op_00
  { ".byte", nullptr, 0 }, // op_01
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "jnc", read_word, 0 }, // op_41
  { "jc", read_word, 0 },  // op_42
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "call", read_word, 2 }, // op_53
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "draw_rectangle", nullptr, 4 }, // op_74
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_78", read_string_bytes, 0 }, // op_78
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "ui_header", read_string_bytes, 0 }, // op_7B
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 }
};

struct script_file {
  unsigned char *bytes = nullptr;
  size_t len = 0;
};

#ifndef nitems
#define nitems(_a) (sizeof((_a)) / sizeof((_a)[0]))
#endif /* nitems */

// Most scripts are small, and this game is from 1989,
// so we can read it all in one shot.
static script_file read_file(const char *fname)
{
  script_file script;

  FILE *fp = fopen(fname, "rb");
  if (fp == nullptr) {
    return script;
  }

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  if (fsize == -1) {
    fclose(fp);
    return script;
  }
  rewind(fp);

  script.bytes = new unsigned char[fsize];
  script.len = (size_t)fsize;

  fread(script.bytes, 1, fsize, fp);

  return script;
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

static int read_string_bytes(const unsigned char *args)
{
  str.len = 0;
  str.string[0] = '\0';

  uint16_t count = extract_string(args, 0, pop_len_string);

  printf("$(\"");
  for (int i = 0; i < str.len; i++) {
    char ch = str.string[i];
    if (ch == '\r') {
      printf("\\r");
    } else {
      printf("%c", ch);
    }
  }
  printf("\")");

  return count;
}

static int read_word(const unsigned char *args)
{
  uint16_t word = *args++;
  word += *args++ << 8;

  printf("0x%04x", word);

  return 2;
}

int main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Invalid argument, needs script file.\n");
    fprintf(stderr, "Example: ./disasm script.bin\n");
    exit(1);
  }

  script_file script = read_file(argv[1]);
  if (script.bytes == nullptr) {
    perror("Unable to read file");
    exit(1);
  }

  printf("Disassembling %s (%zu bytes)\n", argv[1], script.len);

  int num_ops = nitems(op_codes);
  printf("Known ops: %d\n", num_ops);

  unsigned char *iter = script.bytes;
  size_t i = 0;
  while (i < script.len) {
    unsigned char op = *iter;

    if (op > num_ops) {
      fprintf(stderr, "Cannot proceed, unhandled op: 0x%02X, index out of bounds\n", op);
      break;
    }

    op_code *code = &op_codes[op];
    if (code->name == nullptr) {
      fprintf(stderr, "Cannot proceed, unhandled op: 0x%02X\n", op);
      break;
    }

    printf("0x%04X: %s ", (uint16_t)i, code->name);
    iter++;
    i++;

    if (code->func != nullptr) {
      int arg_count = code->func(iter);
      i += arg_count;
      iter += arg_count;
    } else {
      for (int j = 0; j < code->arg_count; j++) {
        unsigned char arg = *iter;
        if (j != 0) {
          printf(", ");
        }
        printf("0x%02x", arg);
        iter++;
        i++;
      }
    }
    printf("\n");
  }

  delete[] script.bytes;

  return 0;
}
