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

/* Routines for working with the timers */

#ifndef DW_TIMER_H
#define DW_TIMER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct timer_ctx {
  uint8_t  timer0; // 0x4C35
  uint8_t  timer1; // 0x4C36
  uint8_t  timer2; // 0x4C37
  uint16_t timer3; // 0x4C38
  uint16_t timer4; // 0x4C3A
  uint8_t  timer5; // 0x4C3C (appears to be more of a flag)
};

extern struct timer_ctx timers;

void initialize_timers();

// 0x4B10
void timer_tick_proc();

#ifdef __cplusplus
}
#endif

#endif /* DW_TIMER_H */
