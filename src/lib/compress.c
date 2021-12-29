/*
 * Copyright (c) 2018 Devin Smith <devin@devinsmith.net>
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
#include <stdlib.h>

#include <compress.h>
#include <utils.h>

struct compress_ctx {
  int counter;
  uint16_t dx;
  int output_idx;

  unsigned char *dict_base;
  unsigned char *dict_ptr;
};

/* Constructs a dictionary from input buffered reader. */
static void build_dictionary(struct buf_rdr *input, struct compress_ctx *ctx)
{
  uint16_t res;
  uint8_t al;
  uint16_t ax = 0;

  ctx->counter--;

  if (ctx->counter < 0) {
    ctx->dx = buf_get16be(input);
    ctx->counter = 15;
  }

  res = ctx->dx << 1;
  if (res < ctx->dx) {
    ctx->dx = res;

    /* Shift overflow */

    if (ctx->counter == 0) {
      al = buf_get8(input);
      ax = (ax & 0xFF00);
      ax += al;
    } else {
      if (ctx->counter >= 8) {
        ctx->counter -= 8;
      } else {
        al = buf_get8(input);
        ax = (al << 8 & 0xFF00) + (ctx->counter & 0xFF00 >> 8);
        ax = ax >> (ctx->counter & 0xFF);
        ctx->dx = ctx->dx | ax;
      }

      /* mov al, dh */
      ax = (ax & 0xFF00);
      ax += (ctx->dx & 0xFF00) >> 8;

      /* mov dh, dl
       * mov dl, ch */
      ctx->dx = (ctx->dx & 0x00FF) << 8;
      ctx->dx += (ctx->counter & 0xFF00) >> 8;
    }
    ax = (ax & 0x00FF);
    ax += (ctx->counter & 0xFF00);

    /* Write 16 bit (word) ax to output buffer in little endian format. */
    *(ctx->dict_ptr + 2) = (ax & 0xff);
    *(ctx->dict_ptr + 3) = (ax & 0xff00) >> 8;

    /* mov al, ah */
    ax = (ax & 0xFF00); // wipe low.
    ax += ((ax & 0xFF00) >> 8);

    /* Write 16 bit (word) ax to output buffer in little endian format. */
    *(ctx->dict_ptr) = (ax & 0xff);
    *(ctx->dict_ptr + 1) = (ax & 0xff00) >> 8;
    return;
  }
  else {
    unsigned char *save_offset;
    ctx->dx = res;
    ctx->output_idx += 4;

    /* write output_idx (16 bits) to output in little endian format. */
    *(ctx->dict_ptr) = (ctx->output_idx & 0xff);
    *(ctx->dict_ptr + 1) = (ctx->output_idx & 0xff00) >> 8;

    save_offset = ctx->dict_ptr;
    ctx->dict_ptr = ctx->dict_base + ctx->output_idx;

    build_dictionary(input, ctx);

    ctx->dict_ptr = save_offset;
    ctx->output_idx += 4;
    *(ctx->dict_ptr + 2) = (ctx->output_idx & 0xff);
    *(ctx->dict_ptr + 3) = (ctx->output_idx & 0xff00) >> 8;
    save_offset = ctx->dict_ptr;

    ctx->dict_ptr = ctx->dict_base + ctx->output_idx;
    build_dictionary(input, ctx);
    ctx->dict_ptr = save_offset;
  }
}

/* Decompress the data into output. */
static void decompress(struct buf_rdr *input, struct compress_ctx *ctx,
    int size, struct buf_wri *output)
{
  uint16_t res;
  uint16_t ax;
  int bx;
  int bp = ctx->counter;
  unsigned char *ptr;

  ctx->counter = size;

  bx = 0;
  while (ctx->counter > 0) {
    ptr = ctx->dict_base + bx;

    /* little endian fetch from dictionary */
    ax = *ptr;
    ax += *(ptr + 1) << 8;

    if (ax != 0) {
      bp--;
      if (bp < 0) {
        ctx->dx = buf_get16be(input);
        bp = 15;
      }
      res = ctx->dx << 1;
      if (res < ctx->dx) {
        /* Carry */
        ctx->dx = res;
        bx = *(ptr + 2);
        bx += *(ptr + 3) << 8;
      } else {
        /* No carry */
        ctx->dx = res;
        bx = ax; /* .loc_4FA5 */
      }

    } else {
      uint8_t al = *(ptr + 2);

      buf_add8(output, al);
      ctx->counter--;
      bx = 0;
    }
  }
}

/*
 * Decompress buffered data from input and write to output. The expected
 * output is written into size.
 *
 * The algorithm used is unknown but it seems to be based on a dictionary.
 * Maybe LZ based or DEFLATE. ?
 *
 */
void decompress_data1(struct buf_rdr *input, struct buf_wri *output, int size)
{
  struct compress_ctx ctx;
  unsigned char *dictionary;

  dictionary = malloc(2048);

  ctx.counter = 0;
  ctx.dx = 0;
  ctx.output_idx = 0;
  ctx.dict_ptr = dictionary;
  ctx.dict_base = dictionary;

  build_dictionary(input, &ctx);
  printf("%d\n", input->offset);
  dump_hex(ctx.dict_base, 64);

  printf("Counter: %d\n", ctx.counter);
  printf("DX: %04x\n", ctx.dx);

  /* Using above dictionary, decompress */
  decompress(input, &ctx, size, output);

  free(dictionary);
}

// Extract "n" bits out of each byte.
// bit_buffer contains leftover bit buffer.
// bits are shifted left, with carry which becomes output.
//
// 0x1D86 -> 1D8C(6)
// 0x1D8A -> 1D8C(5)
// 0x1D8C (num_bits passed in BL)
uint8_t bit_extract(struct bit_extractor *be, int n)
{
  uint8_t al = 0;

  for (int i = 0; i < n; i++) {
    if (be->num_bits == 0) {
      be->bit_buffer = be->data[be->offset];
      be->num_bits = 8;
      be->offset++;
    }
    // 0x1D96
    uint8_t tmp = be->bit_buffer;
    be->bit_buffer = be->bit_buffer << 1;
    be->num_bits--;

    // rcl al, 1
    uint8_t carry = 0;
    if (tmp > be->bit_buffer) {
      carry = 1;
    }
    al = al << 1;
    al += carry;
  }
  return al;
}

// 0x1D2A - 0x1D85
// Characters of the alphabet OR'd with 0x80
static unsigned char alphabet[] = {
        0xa0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xeb, 0xec,
        0xed, 0xee, 0xef, 0xf0, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf9, 0xae,
        0xa2, 0xa7, 0xac, 0xa1, 0x8d, 0xea, 0xf1, 0xf8, 0xfa, 0xb0, 0xb1, 0xb2,
        0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0x30, 0x31, 0x32, 0x33, 0x34,
        0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
        0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53,
        0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0xa8, 0xa9, 0xaf, 0xdc, 0xa3,
        0xaa, 0xbf, 0xbc, 0xbe, 0xba, 0xbb, 0xad, 0xa5
};

// 0x1CF8
uint8_t extract_letter(struct bit_extractor *be)
{
  while (1) {
    uint8_t ret = bit_extract(be, 5);
    if (ret == 0)
      return 0;

    if (ret == 0x1E) {
      // Next byte should be an uppercase letter.

      // stc
      // rcr byte [byte_1CE4], 1
      // rotate carry right bit.
      be->upper_case = be->upper_case >> 1;
      be->upper_case += 0x80;
      continue;
    }

    // 0x1F ?
    if (ret > 0x1E) {
      ret = bit_extract(be, 6);
      ret += 0x1E;
    }

    // ret != 0x1E

    // 0x1D0A
    // offset
    uint8_t al = alphabet[ret - 1];
    be->upper_case = be->upper_case >> 1;

    // If we need an uppercase letter and al is 'a' through 'z'
    if (be->upper_case >= 0x40 && al >= 0xE1 && al <= 0xFA) {
      // Make uppercase.
      al = al & 0xDF;
    }
    // test al, al
    return al;
  }
}
