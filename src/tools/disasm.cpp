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

#include <cctype>
#include <cstdio>
#include <cstdlib>

#include <engine.h>
#include <utils.h>

static int set_byte(const unsigned char *args);
static int set_word(const unsigned char *args);
static int read_string_bytes(const unsigned char *args);
static int read_word(const unsigned char *args);
static int wait_event(const unsigned char *args);
static int read_by_mode(const unsigned char *args);
static int handle_if(const unsigned char *args);
static int read_byte_array_index(const unsigned char *args);
static int op_08(const unsigned char *args);
static int op_0F(const unsigned char *args);
static int op_11(const unsigned char *args);
static int op_12(const unsigned char *args);
static int op_13(const unsigned char *args);
static int op_14(const unsigned char *args);
static int op_1A(const unsigned char *args);
static int op_1C(const unsigned char *args);
static int op_9A(const unsigned char *args);
static int load_resource(const unsigned char *args);
static int for_call(const unsigned char *args);

static bool word_mode = false;

struct op_code {
  const char *name;
  int (*func)(const unsigned char *args);
  int arg_count;
};

op_code op_codes[] = {
  { ".word", set_word, 0 }, // op_00
  { ".byte", set_byte, 0 }, // op_01
  { "op_02", nullptr, 0 }, // op_02
  { "data_res = resource_get(pop_byte())", nullptr, 0 }, // op_03
  { "push_byte(running_script_idx)", nullptr, 0 }, // op_04
  { "word_3AE4 = gamestate[", read_byte_array_index, 1 }, // op_05
  { "set loop =", nullptr, 1 }, // op_06
  { "word_3AE4 = 0", nullptr, 0 }, // op_07
  { "gamestate[", op_08, 1 }, // op_08
  { "word_3AE2 =", read_by_mode, 0 }, // op_09
  { "word_3AE2 = gamestate[", read_byte_array_index, 1 }, // op_0A
  { "word_3AE2 = gamestate[word_3AE4 + ", read_byte_array_index, 1 }, // op_0B
  { "op_0C", read_word, 0 }, // op_0C
  { "op_0D", read_word, 0 }, // op_0D
  { "op_0E", nullptr, 1 }, // op_0E
  { "var offset =", op_0F, 1 }, // op_0F
  { "op_10", nullptr, 2 }, // op_10
  { "gamestate[", op_11, 1 }, // op_11
  { "gamestate[", op_12, 1 }, // op_12
  { "gamestate[word_3AE4 + ", op_13, 1 }, // op_13
  { "word_3ADF[", op_14, 0 }, // op_14
  { "op_15", read_word, 0 }, // op_15
  { "op_16", nullptr, 1 }, // op_16
  { "store_data_resource", nullptr, 1 }, // op_17
  { "op_18", nullptr, 2 }, // op_18
  { "op_19", nullptr, 2 }, // op_19
  { "gamestate[", op_1A, 0 }, // op_1A
  { "op_1B", nullptr, 0 }, // op_1B
  { "word_3ADF[", op_1C, 0 }, // op_1C
  { "memcpy 0x700", nullptr, 0 }, // op_1D
  { "op_1E", nullptr, 0 }, // op_1E
  { "op_1F", nullptr, 0 }, // op_1F
  { "NOP_XXX", nullptr, 0 }, // op_20
  { "op_21", nullptr, 0 }, // op_21
  { "word_3AE2 = word_3AE4", nullptr, 0 }, // op_22
  { "inc game_state[", read_byte_array_index, 1 }, // op_23
  { "inc word_3AE2", nullptr, 0 }, // op_24
  { "inc byte [word_3AE4]", nullptr, 0 }, // op_25
  { "dec game_state[", read_byte_array_index, 1 }, // op_26
  { "dec word_3AE2", nullptr, 0 }, // op_27
  { "dec [mem]", nullptr, 0 }, // op_28
  { "op_29", nullptr, 0 }, // op_29
  { "op_2A", nullptr, 0 }, // op_2A
  { "shl byte [word_3AE4], 1", nullptr, 0 }, // op_2B
  { "op_2C", nullptr, 0 }, // op_2C
  { "word_3AE2 = word_3AE2 >> 1", nullptr, 0 }, // op_2D
  { "op_2E", nullptr, 0 }, // op_2E
  { "op_2F", nullptr, 1 }, // op_2F
  { "op_30", read_by_mode, 0 }, // op_30
  { "op_31", nullptr, 1 }, // op_31
  { "op_32", read_by_mode, 0 }, // op_32
  { "op_33", nullptr, 0 }, // op_33
  { "op_34", nullptr, 1 }, // op_34
  { "op_35", nullptr, 1 }, // op_35
  { "op_36", nullptr, 0 }, // op_36
  { "op_37", nullptr, 0 }, // op_37
  { "word_3AE2 &=", read_by_mode, 0 }, // op_38
  { "op_39", nullptr, 1 },
  { "op_3A", nullptr, 1 }, // op_3A
  { "op_3B", nullptr, 0 }, // op_3B
  { "op_3C", nullptr, 0 }, // op_3C
  { "cmp word_3AE2, gamestate[", read_byte_array_index, 1 }, // op_3D
  { "cmp word_3EA2,", read_by_mode, 0 }, // op_3E
  { "check_gamestate", nullptr, 1 }, // op_3F
  { "cmp word_3AE4,", nullptr, 1 }, // op_40
  { "jnc", read_word, 0 }, // op_41
  { "jc", read_word, 0 },  // op_42
  { "jmp_XXX", read_word, 0 }, // op_43
  { "jz", read_word, 0 }, // op_44
  { "jnz", read_word, 0 }, // op_45
  { "js", read_word, 0 }, // op_46
  { "jns", read_word, 0 }, // op_47 jump not signed
  { "op_48", nullptr, 1 }, // op_48
  { "loop", read_word, 0 }, // op_49
  { "if ", handle_if, 0 }, // op_4A
  { "stc", nullptr, 0 }, // op_4B, set carry
  { "clc", nullptr, 0 }, // op_4C, clear carry
  { "op_4D", nullptr, 0 }, // op_4D
  { "op_4E", nullptr, 0 }, // op_4E
  { "clear_bit game_state", nullptr, 1 }, // op_4F
  { "op_50", nullptr, 0 }, // op_50
  { "op_51", read_word, 0 }, // op_51
  { "jmp", read_word, 0 }, // op_52
  { "call", read_word, 2 }, // op_53
  { "ret", nullptr, 0 }, // op_54
  { "peek_and_pop", nullptr, 0 }, // op_55
  { "push (word|byte)", nullptr, 0 }, // op_56
  { "op_57_res", nullptr, 3 }, // op_57
  { "load_resource", load_resource, 3 }, // op_58, loads 3 bytes, but 1 is byte, 1 is word.
  { "retf", nullptr, 0 }, // op_59, return far
  { "ret", set_byte, 0 }, // op_5A, hack for setting byte mode
  { "op_5B", nullptr, 0 }, // op_5B
  { "var al = game_state[6]", for_call, 0 }, // op_5C
  { "word_3AE2 = get_char_data", nullptr, 1 }, // op_5D
  { "set_char_prop", nullptr, 1 }, // op_5E
  { "op_5F", nullptr, 0 }, // op_5F
  { "op_60", nullptr, 0 }, // op_60
  { "test_player_property", nullptr, 1 }, // op_61
  { "op_62", nullptr, 0 }, // op_62
  { "op_63", read_word, 0 }, // op_63
  { "op_64", nullptr, 0 }, // op_64
  { "op_65", nullptr, 0 }, // op_65
  { "test game_state[", read_byte_array_index, 1 }, // op_66
  { "op_67", nullptr, 0 }, // op_67
  { "op_68", nullptr, 1 }, // op_68
  { "op_69", nullptr, 1 }, // op_69
  { "op_6A", nullptr, 4 }, // op_6A
  { "op_6B", nullptr, 0 }, // op_6B
  { "op_6C", nullptr, 0 }, // op_6C
  { "op_6D", nullptr, 0 }, // op_6D
  { nullptr, nullptr, 0 },
  { "op_6F", nullptr, 1 }, // op_6F
  { "op_70", nullptr, 0 }, // op_70
  { "load_world", nullptr, 0 }, // op_71
  { nullptr, nullptr, 0 },
  { "op_73", nullptr, 0 }, // op_73
  { "draw_rectangle", nullptr, 4 }, // op_74
  { "draw_ui_full", nullptr, 0 }, // op_75
  { "draw_pattern", nullptr, 0 }, // op_76
  { "draw_and_set", read_string_bytes, 0 }, // op_77
  { "set_msg", read_string_bytes, 0 }, // op_78
  { nullptr, nullptr, 0 },
  { "extract_string", nullptr, 0 }, // op_7A
  { "ui_header", read_string_bytes, 0 }, // op_7B
  { "ui_header_random_encounter", nullptr, 0 }, // op_7C
  { "write_character_name", nullptr, 0 }, // op_7D
  { "op_7E", nullptr, 1 }, // op_7e: XXX ?
  { nullptr, nullptr, 0 },
  { "advance_cursor", nullptr, 1 }, // op_80
  { "op_81 word_3AE2", nullptr, 0 }, // op_81
  { "op_82", nullptr, 1 }, // op_82
  { "write_number", nullptr, 0 }, // op_83
  { "malloc(word_3AE2)", nullptr, 0 }, // op_84
  { "resource_release", nullptr, 0 }, // op_85
  { "word_3AE2 = load_resource(word_3AE2)", nullptr, 0 }, // op_86
  { nullptr, nullptr, 0 },
  { "wait_escape", nullptr, 0 }, // op_88
  { "wait_event", wait_event, 0 }, // op_89
  { "random_encounter?", nullptr, 0 }, // op_8A
  { "refresh_viewport", nullptr, 0 }, // op_8B
  { "prompt 'Y', 'N'", nullptr, 0 }, // op_8C
  { "op_8D", nullptr, 0 }, // op_8D: Read string??
  { nullptr, nullptr, 0 },
  { "op_8F", nullptr, 0 }, // op_8F
  { "op_90", nullptr, 1 }, // op_90
  { "op_91", nullptr, 0 }, // op_91
  { "op_92", nullptr, 0 }, // op_92
  { "push_byte(word_3AE4 & 0xFF)", nullptr, 0 }, // op_93
  { "pop byte", nullptr, 0 }, // op_94
  { "ui_draw_string", nullptr, 0 }, // op_95
  { "draw_padded_string", nullptr, 0 }, // op_96
  { "load_char_data", nullptr, 1 }, // op_97
  { "op_98", nullptr, 1 }, // op_98
  { "test word_3AE2", nullptr, 0 }, // op_99
  { "gamestate[", op_9A, 1 }, // op_9A
  { "op_9B", nullptr, 2 }, // op_9B
  { "op_9C", nullptr, 0 }, // op_9C
  { "op_9D", nullptr, 2 }, // op_9D
  { "op_9E", nullptr, 0 }, // op_9E
  { "op_9F", nullptr, 0 }, // op_9F
  { "op_A0", nullptr, 0 }, // op_A0
  { nullptr, nullptr, 0 },
  { "op_A2", nullptr, 0 }, // op_A2
  { nullptr, nullptr, 0 },
  { "op_A4", nullptr, 0 }, // op_A4
  { "op_A5", nullptr, 0 }, // op_A5 ?
  { "op_A6", nullptr, 0 }, // op_A6
  { "op_A7", nullptr, 0 }, // op_A7
  { "op_A8", nullptr, 0 }, // op_A8
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_AC", nullptr, 0 }, // op_AC
  { nullptr, nullptr, 0 },
  { "op_AE", nullptr, 0 }, // op_AE
  { "op_AF", nullptr, 0 }, // op_AF
  { "op_B0", nullptr, 0 }, // op_B0
  { nullptr, nullptr, 0 },
  { "op_B2", nullptr, 0 }, // op_B2
  { "op_B3", nullptr, 0 }, // op_B3
  { nullptr, nullptr, 0 },
  { "op_B5", nullptr, 0 }, // op_B5
  { nullptr, nullptr, 0 },
  { "op_B7", nullptr, 0 }, // op_B7
  { "op_B8", nullptr, 0 }, // op_B8
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_BC", nullptr, 0 }, // op_BC
  { "op_BD", nullptr, 0 }, // op_BD
  { "op_BE", nullptr, 0 }, // op_BE
  { nullptr, nullptr, 0 },
  { "op_C0", nullptr, 0 }, // op_C0
  { "op_C1", nullptr, 0 }, // op_C1
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_C5", nullptr, 0 }, // op_C5
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_C8", nullptr, 0 }, // op_C8
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_CF", nullptr, 0 }, // op_CF
  { "op_D0", nullptr, 0 }, // op_D0
  { "op_D1", nullptr, 0 }, // op_D1
  { "op_D2", nullptr, 0 }, // op_D2
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_D6", nullptr, 0 }, // op_D6
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_DA", nullptr, 0 }, // op_DA
  { nullptr, nullptr, 0 },
  { "op_DC", nullptr, 0 }, // op_DC
  { nullptr, nullptr, 0 },
  { "op_DE", nullptr, 0 }, // op_DE
  { nullptr, nullptr, 0 },
  { "op_E0", nullptr, 0 }, // op_E0
  { "op_E1", nullptr, 0 }, // op_E1
  { "op_E2", nullptr, 0 }, // op_E2
  { nullptr, nullptr, 0 },
  { "op_E4", nullptr, 0 }, // op_E4
  { "op_E5", nullptr, 0 }, // op_E5
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_E8", nullptr, 0 }, // op_E8
  { nullptr, nullptr, 0 },
  { "op_EA", nullptr, 0 }, // op_EA
  { "op_EB", nullptr, 0 }, // op_EB
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_F0", nullptr, 0 }, // op_F0
  { "op_F1", nullptr, 0 }, // op_F1
  { nullptr, nullptr, 0 },
  { "op_F3", nullptr, 0 }, // op_F3
  { "op_F4", nullptr, 0 }, // op_F4
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_F8", nullptr, 0 }, // op_F8
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { nullptr, nullptr, 0 },
  { "op_FC", nullptr, 0 }, // op_FC
  { "op_FD", nullptr, 0 }, // op_FD
  { "op_FE", nullptr, 0 }, // op_FE
  { "nop", nullptr, 0 } // op_FF
};

struct script_file {
  unsigned char *bytes = nullptr;
  size_t len = 0;
};

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
  script.len = fread(script.bytes, 1, fsize, fp);

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

static int for_call(const unsigned char *args)
{
  uint16_t word = *args++;
  word += *args++ << 8;

  printf("\n        for (game_state[6] < game_state[0x1F]; game_state[6]++) {");
  printf("\n          call 0x%04X", word);
  printf("\n        }");
  printf("\n        game_state[6] = al");

  word_mode = false;

  return 2;
}

static int wait_event(const unsigned char *args)
{
  int count = 0;
  uint16_t word;

  word = *args++;
  word += *args++ << 8;
  printf(" 0x%04X", word);

  uint8_t ch = *args++;
  count = 2;

  const char *key_desc = nullptr;

  while (ch != 0xff) {
    bool printable = isprint(ch & 0x7f);
    switch (ch) {
    case 0:
      printable = true;
      key_desc = "Unknown";
      break;
    case 1:
      printable = true;
      key_desc = "#[1-7]";
      break;
    case 0x9B:
      printable = true;
      key_desc = "ESC";
      break;
    case 0x88:
      printable = true;
      key_desc = "LEFT";
      break;
    case 0x8A:
      printable = true;
      key_desc = "DOWN";
      break;
    case 0x8B:
      printable = true;
      key_desc = "UP";
      break;
    case 0x8D:
      printable = true;
      key_desc = "ENTER";
      break;
    case 0x95:
      printable = true;
      key_desc = "RIGHT";
      break;
    default:
      key_desc = nullptr;
      break;
    }

    if (!printable) {
      printf(", 0x%02x", ch);
    } else {
      if (key_desc != nullptr) {
        printf(", \"%s\",", key_desc);
      } else {
        printf(", '%c',", ch & 0x7f);
      }
      read_word(args);
      args += 2;
      count += 2;
    }
    ch = *args++;
    count++;
  }
  printf(", 0xff");
  count++;

  return count;
}

static int set_word(const unsigned char *args)
{
  (void)args; // unused

  word_mode = true;
  return 0;
}

static int set_byte(const unsigned char *args)
{
  (void)args; // unused

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
  printf("++byte_3AE4 != 0x%02X ", ch);
  uint16_t word = *args++;
  word += *args++ << 8;

  printf("jmp 0x%04x", word);

  return 3;
}

static int read_byte_array_index(const unsigned char *args)
{
  unsigned char ch = *args++;
  printf("0x%02X]", ch);

  return 1;
}

static int op_08(const unsigned char *args)
{
  printf("0x%02x] = word_3AE4", *args++);
  return 1;
}

static int op_0F(const unsigned char *args)
{
  unsigned char ch = *args++;

  printf(" gamestate[0x%02X]\n", ch);
  printf("        offset += gamestate[0x%02X] << 8\n", ch + 1);
  printf("        offset += word_3AE4\n");
  printf("        word_3AE2 = resource_idx(gamestate[0x%02X])->bytes[offset]\n", ch + 2);
  printf("        word_3AE2 += resource_idx(gamestate[0x%02X])->bytes[offset + 1] << 8", ch + 2);

  return 1;
}

static int op_14(const unsigned char *args)
{
  uint16_t word = *args++;
  word += *args++ << 8;

  printf("0x%04x] = word_3AE2", word);

  if (word_mode)
    printf("\n        word_3ADF[0x%04X] = (word_3AE2 & 0xFF00) >> 8", word + 1);

  return 2;
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

static int op_1C(const unsigned char *args)
{
  uint16_t word = *args++;
  word += *args++ << 8;

  printf("0x%04x] = ", word);
  printf("0x%02x", *args++);

  if (word_mode)
    printf("\n        word_3ADF[0x%02X] = 0x%02x", word + 1, *args++);

  return word_mode ? 4 : 3;
}

static int op_11(const unsigned char *args)
{
  unsigned char idx = *args++;
  printf("0x%02x] = 0", idx);

  if (word_mode)
    printf("\n        gamestate[0x%02X] = 0", idx + 1);

  return 1;
}

static int op_9A(const unsigned char *args)
{
  unsigned char idx = *args++;
  printf("0x%02x] = 0xFF", idx);

  if (word_mode)
    printf("\n        gamestate[0x%02X] = 0xFF", idx + 1);

  return 1;
}

static int op_12(const unsigned char *args)
{
  printf("0x%02x] = word_3AE2", *args++);
  return 1;
}

static int op_13(const unsigned char *args)
{
  printf("0x%02X] = word_3AE2", *args++);
  return 1;
}

static int load_resource(const unsigned char *args)
{
  uint8_t res = *args++;
  uint16_t offset = *args++;
  offset += *args++ << 8;

  printf(" res: 0x%02x, ", res);
  printf("offset: 0x%04x", offset);

  return 3;
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(stderr, "Invalid argument, needs script file.\n");
    fprintf(stderr, "Example: ./disasm script.bin\n");
    exit(1);
  }

  script_file script = read_file(argv[1]);
  if (script.bytes == nullptr) {
    perror("Unable to read file");
    exit(1);
  }

  size_t offset = 0;
  if (argc == 3) {
    offset += atoi(argv[2]);
  }

  printf("Disassembling %s (%zu bytes)\n", argv[1], script.len);

  unsigned char *iter = script.bytes + offset;
  size_t i = offset;
  while (i < script.len) {
    unsigned char op = *iter;

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
