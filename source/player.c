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

#include "player.h"

// This should be 512 bytes long.
struct player_record {
  // first few bytes are the name.
  unsigned char data[512];
};

// In the dragon.com implementation this occupies 0E73:0000-0DFF, but in the
// COM file it's at 0x1DD:C960 (where CS = 0x1DD)
//
// This is because it is calculated as
// ax = 0xC960 >> 4
// mov bx, cs
// add ax, bx
//
// Where CS is 0x1DD
//    0xE73 -     (0xC960 >> 4) + 0x1DD
//
// 01DD:C960 -> 0E73:0000
//
// This is character data. A Dragon Wars party can be 7 people.
// Each character uses 512 bytes (0x200) so 512 * 7 = 0xE00
static unsigned char data_C960[0xE00] = { 0 };

#define SIZE_OF_PLAYER 512

unsigned char *get_player_data_base()
{
  return data_C960;
}

unsigned char *get_player_data(int player)
{
  size_t offset = player * SIZE_OF_PLAYER;
  return data_C960 + offset;
}
