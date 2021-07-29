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

// Tests for bitextract

#include <cstdlib>

#include <check.h>

#include "bithelp.h"

START_TEST(test_be_basic)
{
  unsigned char test_data[] = { 0xf2 };

  bit_extractor be { 0 };
  be.data = test_data;
  uint8_t val = bit_extract(&be, 5);
  ck_assert_int_eq(val, 0x1e);
  // Bytes left over: 3
  ck_assert_int_eq(be.num_bits, 3);
  // Advanced 1 byte.
  ck_assert_int_eq(be.offset, 1);
  // Buffer contains left over.
  ck_assert_int_eq(be.bit_buffer, (uint8_t)(test_data[0] << 5));
}
END_TEST

START_TEST(test_be_multibyte)
{
    unsigned char test_data[] = { 0xf2, 0x9d };

    bit_extractor be { 0 };
    be.data = test_data;
    uint8_t val = bit_extract(&be, 5);
    ck_assert_int_eq(val, 0x1e);
    ck_assert_int_eq(be.num_bits, 3);
    ck_assert_int_eq(be.offset, 1);
    ck_assert_int_eq(be.bit_buffer, (uint8_t)(test_data[0] << 5));

    val = bit_extract(&be, 5);
    ck_assert_int_eq(val, 0x0A);
    ck_assert_int_eq(be.num_bits, 6);
    ck_assert_int_eq(be.offset, 2);
    ck_assert_int_eq(be.bit_buffer, (uint8_t)(test_data[1] << 2));

}
END_TEST


Suite* bitinfo_suite()
{
  Suite *s = suite_create("BitExtractor");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_be_basic);
  tcase_add_test(tc_core, test_be_multibyte);
  suite_add_tcase(s, tc_core);

  return s;
}

int main(int argc, char *argv[])
{
  Suite *s = bitinfo_suite();
  SRunner *sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);
  int number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
