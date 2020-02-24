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

#include "engine.h"
#include "resource.h"
#include "utils.h"

/* Represents the engine that Dragon wars runs.
 *
 * It appears that Dragon Wars executes a script where each op code
 * does some action.
 *
 * Lots of global variables here until we can figure out how they are used. */


uint8_t byte_3AE1 = 0;
uint16_t word_3AE2 = 0;

// x86 CPU registers
uint16_t cpu_ax;

// 0x3B0E
static void op_01(void)
{
  word_3AE2 = (cpu_ax & 0xFF00);
  byte_3AE1 = (cpu_ax & 0xFF00) >> 8;
}

void run_engine()
{
  // 0x1A6
  // Loads into 0x1887:0000
  struct resource code_res;
  if (resource_load(RESOURCE_UNKNOWN, &code_res) != 0)
  {
    printf("Failed to load unknown resource\n");
  }
  printf("Resource bytes: %zu\n", code_res.len);
  dump_hex(code_res.bytes, 0x80);

  unsigned char *op_code = code_res.bytes;

  int done = 0;
  cpu_ax = 0;

  // 0x3AA0
  while (!done) {
    switch (*op_code) {
    case 0x01:
      op_01();
      break;
    default:
      printf("Unhandled op code: 0x%02X\n", *op_code);
      done = 1;
      break;
    }
    op_code++;
  }
}

