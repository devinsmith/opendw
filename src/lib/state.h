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

#ifndef DW_STATE_H
#define DW_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

// Directions
enum {
  DIRECTION_NORTH = 0,
  DIRECTION_EAST = 1,
  DIRECTION_SOUTH = 2,
  DIRECTION_WEST = 3
};

// Managing game state.
//


// We should break this apart.
struct game_state {
  // 0x00 - X position of player
  // 0x01 - Y position of player
  // 0x02 - Current world number? (e.g. Purgatory?)
  // 0x03 - Current direction (0 = North, 1 = East, 2 = South, 3 = West)
  // 0x06 - Currently selected player.
  // 0x1F - Number of characters in the party.
  // 0x56 - Script index?
  // 0x6A - 0x6D (Gold)
  // 0x6E - 0x71 (Experience)
  // 0xC6 - 0x?? - New character name.
  unsigned char unknown[256];
};

extern struct game_state game_state;

void set_game_state(const char *func_src, int offset, unsigned char value);

#ifdef __cplusplus
}
#endif

#endif /* DW_STATE_H */
