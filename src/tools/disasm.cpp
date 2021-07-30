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
#include <utils.h>

static int set_byte(const unsigned char *args);
static int set_word(const unsigned char *args);
static int read_string_bytes(const unsigned char *args);
static int read_word(const unsigned char *args);
static int wait_event(const unsigned char *args);
static int read_by_mode(const unsigned char *args);
static int handle_if(const unsigned char *args);
static int op_0A(const unsigned char *args);
static int op_1A(const unsigned char *args);
static int op_11(const unsigned char *args);
static int op_12(const unsigned char *args);

static bool word_mode = false;

struct op_code {
  const char *name;
  int (*func)(const unsigned char *args);
  int arg_count;
};

op_code op_codes[] = {
  { ".word", set_word, 0 }, // op_00
  { ".byte", set_byte, 0 }, // op_01
  { nullptr, nullptr, 0 }, // op_02
  { "op_03", nullptr, 0 }, // op_03
  { "push byte_3AE9", nullptr, 0 }, // op_04
  { "set_gamestate", nullptr, 1 }, // op_05
  { "set loop =", nullptr, 1 }, // op_06
  { "word_3AE4 = 0", nullptr, 0 }, // op_07
  { "set_gamestate: idx =", nullptr, 1 }, // op_08
  { "word_3AE2 =", read_by_mode, 0 }, // op_09
  { "word_3AE2 = gamestate[", op_0A, 1 }, // op_0A
  { "op_0B", nullptr, 1 }, // op_0B
  { "op_0C", read_word, 0 }, // op_0C
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_0F", nullptr, 1 }, // op_0F
  { nullptr, nullptr, 0 },
  { "gamestate[", op_11, 1 }, // op_11
  { "gamestate[", op_12, 1 }, // op_12
  { "op_13", nullptr, 1 }, // op_13
  { "op_14", read_word, 0 }, // op_14
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "store_data_resource", nullptr, 1 }, // op_17
  { nullptr, nullptr, 0 },
  { "op_19", nullptr, 2 }, // op_19
  { "gamestate[", op_1A, 0 },
  { nullptr, nullptr, 0 },
  { "op_1C", op_1A, 0 }, // op_1C
  { "memcpy 0x700", nullptr, 0 }, // op_1D
  { nullptr, nullptr, 0 },
  { "op_1F", nullptr, 0 }, // op_1F
  { nullptr, nullptr, 0 }, // op_20
  { "op_21", nullptr, 0 }, // op_21
  { nullptr, nullptr, 0 },
  { "inc [mem]", nullptr, 1 }, // op_23
  { nullptr, nullptr, 0 },
  { "inc reg", nullptr, 0 }, // op_25
  { "op_26", nullptr, 1 }, // op_26
  { "op_27", nullptr, 0 }, // op_27
  { "dec [mem]", nullptr, 0 }, // op_28
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_30", read_by_mode, 0 }, // op_30
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_38", read_by_mode, 0 }, // op_38
  { nullptr, nullptr, 0 },
  { "op_3A", nullptr, 1 }, // op_3A
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_3E", read_by_mode, 0 }, // op_3E
  { "check_gamestate", nullptr, 1 }, // op_3F
  { "op_40", nullptr, 1 }, // op_40
  { "jnc", read_word, 0 }, // op_41
  { "jc", read_word, 0 },  // op_42
  { nullptr, nullptr, 0 },
  { "jnz", read_word, 0 }, // op_44
  { "jz", read_word, 0 }, // op_45
  { "js", read_word, 0 }, // op_46
  { "jns", read_word, 0 }, // op_47 jump not signed
  { nullptr, nullptr, 0 },
  { "loop", read_word, 0 }, // op_49
  { "if ", handle_if, 0 }, // op_4A
  { "stc", nullptr, 0 }, // op_4B, set carry
  { "clc", nullptr, 0 }, // op_4C, clear carry
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "jmp", read_word, 0 }, // op_52
  { "call", read_word, 2 }, // op_53
  { "ret", nullptr, 0 }, // op_54
  { "peek_and_pop", nullptr, 0 }, // op_55
  { "push (word|byte)", nullptr, 0 }, // op_56
  { "op_57_res", nullptr, 3 }, // op_57
  { "load_resource", nullptr, 3 }, // op_58, loads 3 bytes, but 1 is byte, 1 is word.
  { nullptr, nullptr, 0 },
  { "pop 3words", nullptr, 0 }, // op_5A
  { nullptr, nullptr, 0 },
  { "op_5C", read_word, 0 }, // op_5C
  { "get_char_data", nullptr, 1 }, // op_5D
  { "set_char_prop", nullptr, 1 }, // op_5E
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_66", nullptr, 1 }, // op_66
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_69", nullptr, 1 }, // op_69
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
  { "draw_ui_full", nullptr, 0 }, // op_74
  { nullptr, nullptr, 0 },
  { "draw_and_set", read_string_bytes, 0 }, // op_77
  { "set_msg", read_string_bytes, 0 }, // op_78
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "ui_header", read_string_bytes, 0 }, // op_7B
  { nullptr, nullptr, 0 },
  { "write_character_name", nullptr, 0 }, // op_7D
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_80", nullptr, 1 }, // op_80
  { "op_81", nullptr, 0 }, // op_81
  { nullptr, nullptr, 0 },
  { "write_number", nullptr, 0 }, // op_83
  { nullptr, nullptr, 0 },
  { "resource_release", nullptr, 0 }, // op_85
  { "word_3AE2 = load_resource(word_3AE2)", nullptr, 0 }, // op_86
  { nullptr, nullptr, 0 },
  { "wait_escape", nullptr, 0 }, // op_88
  { "wait_event", wait_event, 0 }, // op_89, XXX: Sometimes takes 3 args?
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "prompt 'Y', 'N'", nullptr, 0 }, // op_8C
  { "op_8D", nullptr, 0 }, // op_8D: Read string??
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_93", nullptr, 0 }, // op_93
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "load_char_data", nullptr, 1 }, // op_97
  { "op_98", nullptr, 1 }, // op_98
  { "test word_3AE2", nullptr, 0 }, // op_99
  { "op_9A", nullptr, 1 }, // op_9A
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_9E", nullptr, 0 }, // op_9E
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

  printf(" $(\"");
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

  printf(" 0x%04x", word);

  return 2;
}

static int wait_event(const unsigned char *args)
{
  int count = 0;

  //dump_hex(args, 32);
  //printf("\n");
  //exit(1);

  // XXX: This seems to be a variable number of arguments,
  // this can't be correct.
  read_word(args);
  printf(", ");
  args += 2;
  printf("%c, ", *args++ & 0x7f);
  read_word(args);
  printf(", ");
  args += 2;
  printf("%c, ", *args++ & 0x7f);
  read_word(args);
  args += 2;
  count = 8;

  // Read until we reach 0xff
  unsigned char ch = *args++;
  while (ch != 0xff) {
    printf(", 0x%02X", ch);
    count++;
    ch = *args++;
  }
  count++;
  printf(", 0x%02x", ch);

  return count;
}

static int set_word(const unsigned char *args)
{
  word_mode = true;
  return 0;
}

static int set_byte(const unsigned char *args)
{
  word_mode = false;
  return 0;
}

static int read_by_mode(const unsigned char *args)
{
  if (word_mode) {
    uint16_t word = *args++;
    word += *args++ << 8;

    printf(" 0x%04x", word);
  } else {
    printf(" 0x%02x", *args++);
  }

  return word_mode ? 2 : 1;
}

static int handle_if(const unsigned char *args)
{
  unsigned char ch = *args++;
  printf("byte_3AE4 != 0x%02X ", ch);
  uint16_t word = *args++;
  word += *args++ << 8;

  printf("jmp 0x%04x", word);

  return 3;
}

static int op_0A(const unsigned char *args)
{
  unsigned char ch = *args++;
  printf("0x%02X]", ch);

  return 1;
}

static int op_1A(const unsigned char *args)
{
  unsigned char idx = *args++;
  printf("0x%02x] = ", idx);
  printf("0x%02x", *args++);

  if (word_mode)
    printf("\n        gamestate[0x%02X] = 0x%02x", idx + 1, *args++);

  return word_mode ? 3 : 2;
}

static int op_11(const unsigned char *args)
{
  unsigned char idx = *args++;
  printf("0x%02x] = 0", idx);

  if (word_mode)
    printf("\n        gamestate[0x%02X] = 0", idx + 1);

  return 1;
}

static int op_12(const unsigned char *args)
{
  printf("0x%02x] = word_3AE2", *args++);
  return 1;
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

    printf("0x%04X: %s", (uint16_t)i, code->name);
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
          printf(",");
        }
        printf(" 0x%02x", arg);
        iter++;
        i++;
      }
    }
    printf("\n");
  }

  delete[] script.bytes;

  return 0;
}
