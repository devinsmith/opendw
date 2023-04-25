/*
 * Copyright (c) 2021 Devin Smith <devin@devinsmith.net>
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

#include "log.h"
#include "state.h"

// The game state is a 256 byte "scratch/work" area for the game engine
// to manage and keep track of various aspects of the game.
//
// It starts at address 0x3860
struct game_state game_state = {0};

// Offsets 33, 34, 35, 36 hold first 4 bytes of level data.

void set_game_state(const char *func_src, int offset, unsigned char value)
{
  log_trace("%s: [%d] = 0x%02X (%s)", __func__, offset, value, func_src);
  if (offset == 31) {
    log_trace("   SETTING MONSTER?\n");
  }
  if (offset == 0x47) {
    log_trace("   HMM?");
  }
  if (offset == 65) { // 0x41
    log_trace("   SETTING MONSTER?\n");
  }
  if (offset == 88) { // 0x58
    log_trace("   SETTING MONSTER?\n");
  }
  if (offset == 0xBE) {
    log_trace("   SETTING Direction to %d\n", value);
  }
  game_state.unknown[offset] = value;
}

unsigned char get_game_state(const char *func_src, int offset)
{
  log_trace("%s: [%d] (%s)", __func__, offset, func_src);
  return game_state.unknown[offset];
}
