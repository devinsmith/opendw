/*
 * Copyright (c) 2020-2023 Devin Smith <devin@devinsmith.net>
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

/* Routines for working with the mouse */

#include <stdio.h>
#include <stdlib.h>

#include "mouse.h"
#include "vga.h"
#include "ui.h"

// 0x246D
// Indicates the type of mouse cursor to use.
uint16_t mouse_cursor_idx;

// 0x2476
// Indicates whether mouse cursor is visible?
unsigned char mouse_cursor_visible = 0;

// 0x3854 - 0x3859
struct mouse_status mouse;

// Mouse cursors (see mouse.cpp in tools)
uint16_t data_6470[] = { 0x647C, 0x64EE, 0x6654, 0x65D2, 0x66C6, 0x6550 };

static void sub_2061();

// 0x1F10
void mouse_show_cursor()
{
  if (mouse.enabled == 0) {
    return;
  }

  if (mouse_cursor_visible == 0) {
    // 0x1F26
    sub_2061(); // Draw mouse cursor
  } else {
    // 0x1F1E
  }

  if (mouse_cursor_visible != 0) {
    //sub_1F38();
  }

  // 0x1F26
  printf("%s: 0x1F17 unimplemented\n", __func__);
  exit(1);
}

// Restores saved screen buffer where cursor was.
// 0x1F54
void mouse_restore_screen_buffer(uint8_t al)
{
  if (mouse_cursor_visible == 0) {
    return;
  }

  // 0x1F5B
  printf("%s: 0x1F5B unhandled.\n", __func__);
  exit(1);
}

// 0x1F8F
void mouse_disable_cursor()
{
  if (mouse_cursor_visible == 0)
    return;

  printf("%s: 0x1F96 unimplemented\n", __func__);
  exit(1);
}

// 0x2061
static void sub_2061()
{
  uint16_t si = data_6470[mouse_cursor_idx];

  //sub_23A1();

  mouse_cursor_visible = 0xff;
}

// 0x2AEE
// Check if mouse is inbounds on a rectangle?
// Side effects, set's carry flag and also mouse cursor
int sub_2AEE(uint16_t flags)
{
  uint16_t ax;
  int cf;

  mouse_cursor_idx = 2;
  ax = mouse.x;
  ax = ax << 3;

  if (ax > draw_rect.x) {
    printf("%s: 0x2BO2 unimplemented\n", __func__);
    exit(1);
  }

  if ((flags & 0x04) != 0) {
    ax = ax & 0xFF00;
    mouse_cursor_idx = 0;
    cf = 1;
    return 1;
  }
  if ((flags & 0x10) != 0) {
    ax = mouse.x;
    if (ax >= 0xD8) {
      // 0x2B4B
      printf("%s: 0x2B4B unimplemented\n", __func__);
      exit(1);
    }
  }
  // 0x2B81
  if ((flags & 0x20) != 0) {
    ax = mouse.x;
    if (ax >= 0x10) {
      printf("%s: 0x2B8E unimplemented\n", __func__);
      exit(1);
    }
  }

  // 0x2BCF
  return 0;
}

// 0x3824
void poll_mouse()
{
  // No support for reading mouse position at this point.
  // This is determined by 0x3855.

  mouse.enabled = 0;
  mouse.x = 0;
  mouse.y = 0;
  mouse.clicked = 0;
}

// 0x3840
uint8_t mouse_get_clicked()
{
  // Mouse clicked will store the last 2 clicks in the high bits
  // of mouse.clicked.
  return mouse.clicked & 0xC0;
}


