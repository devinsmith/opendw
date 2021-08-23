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

// Tests for vga.c

#include "test_vga.h"

#include "vga.h"

START_TEST(test_vga_keyb)
{
  vga_initialize(64, 64);

  for (int i = 0; i < 32; i++) {
    vga_addkey(i);
  }

  for (int i = 0; i < 32; i++) {
    int actual = vga_getkey2();
    ck_assert_int_eq(actual, i);
  }


  int key = vga_getkey2();
  ck_assert_int_eq(key, 0);

  for (int i = 0; i < 32; i++) {
    vga_addkey(i);
  }

  key = vga_getkey2();
  ck_assert_int_eq(key, 0);

  vga_end();
}
END_TEST

Suite* vga_suite()
{
  Suite *s = suite_create("vga");

  TCase *tc_core = tcase_create("core");
  tcase_add_test(tc_core, test_vga_keyb);
  suite_add_tcase(s, tc_core);

  return s;
}

