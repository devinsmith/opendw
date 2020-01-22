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

struct viewport_data {
  int xpos;
  int ypos;
  int runlength;
  int numruns;
  int unknown1;
  int unknown2;
  unsigned char *data;
};


unsigned char viewport_tl[] = {
  0xAA, 0xAA, 0xAA, 0x06, // 0x00 (0x675C-0x675F)
  0xAA, 0xAA, 0xAA, 0x06, // 0x01 (0x6760-0x6763)
  0xAA, 0xAE, 0xE2, 0x06, // 0x02 (0x6764-0x6767)
  0x00, 0x00, 0x00, 0x66, // 0x03 (0x6768-0x676B)
  0x22, 0x22, 0x20, 0x66, // 0x04 (0x676C-0x676F)
  0x0E, 0xAA, 0xA0, 0x66, // 0x05 (0x6770-0x6773)
  0x0E, 0xAE, 0x06, 0x66, // 0x06 (0x6774-0x6777)
  0x0E, 0xE0, 0x66, 0x66, // 0x07 (0x6778-0x677B)
  0x0A, 0x06, 0x66, 0x66, // 0x08 (0x677C-0x677F)
  0x06, 0x66, 0x66, 0x66  // 0x09 (0x6780-0x6783)
};

struct viewport_data viewports[] = {
  {
    0x00, 0x00, 0x04, 0x0A, 0x00, 0x00,
    viewport_tl
  }
};
