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

#include "timers.h"

struct timer_ctx timers;

void initialize_timers()
{
  timers.timer0 = 1;
  timers.timer1 = 1;
  timers.timer2 = 1;
  timers.timer3 = 1;
  timers.timer4 = 1;
  timers.timer5 = 0;
}

// 0x4B10
void timer_tick_proc()
{
  if (timers.timer2 > 0) {
    timers.timer2--;
  }

  if (timers.timer4 > 0) {
    timers.timer4--;
  }
}

