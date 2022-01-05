/*
 * Copyright (c) 2018-2020 Devin Smith <devin@devinsmith.net>
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

#ifndef DW_VGA_INTERFACE_H
#define DW_VGA_INTERFACE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct vga_driver {
  const char *driver_name;
  int (*initialize)(int game_width, int game_height);
  void (*end)();
  void (*update)();
  void (*waitkey)();
  uint8_t* (*memory)();
  uint16_t (*getkey)();
  int (*poll)();
};

struct mouse_status {
  // clicked contains the last 2 left mouse clicks
  // (in order to track double clicks)
  // as the most significant bytes
  //
  // The value of clicked could then be:
  //  0 -> 00000000b (no button clicked)
  // 80 -> 10000000b (single click)
  // C0 -> 11000000b (double click)
  uint8_t clicked; // 0x3854

  uint8_t enabled; // 0x3855 (might be enabled or num of buttons)

  uint16_t x; // 0x3556
  uint16_t y; // 0x3558
};

void register_vga_driver(struct vga_driver *driver);

int vga_initialize(int game_width, int game_height);
uint8_t* vga_memory();
void vga_update();
uint16_t vga_getkey();
void vga_waitkey();
void vga_end();
void vga_addkey(int key);
int vga_getkey2();
void vga_poll_events();

// Intended for use by various vga drivers.
void video_setup();

#ifdef __cplusplus
}
#endif

#endif /* DW_VGA_INTERFACE_H */
