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

