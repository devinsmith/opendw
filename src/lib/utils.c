/*
 * Copyright (c) 2016 Devin Smith <devin@devinsmith.net>
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
#include <string.h>

#include <ctype.h>

#include "utils.h"

void dump_hex(const void *vp, size_t len)
{
  char linebuf[80];
  int i;
  int linebuf_dirty = 0;
  const unsigned char *p = (const unsigned char *)vp;

  memset(linebuf, ' ', sizeof(linebuf));
  linebuf[70] = '\0';

  for (i = 0; i < len; i++) {
    int x = i % 16;
    int ch = (unsigned)p[i];
    char hex[20];

    if (x >= 8)
      x = x * 3 + 1;
    else
      x = x * 3;
    snprintf(hex, sizeof(hex), "%02x", ch);
    linebuf[x] = hex[0];
    linebuf[x + 1] = hex[1];

    if (isprint(ch))
      linebuf[52 + (i % 16)] = ch;
    else
      linebuf[52 + (i % 16)] = '.';

    linebuf_dirty = 1;
    if (!((i + 1) % 16)) {
      fprintf(stderr, "%s\n", linebuf);
      memset(linebuf, ' ', sizeof(linebuf));
      linebuf[70] = '\0';
      linebuf_dirty = 0;
    }
  }
  if (linebuf_dirty == 1)
    fprintf(stderr, "%s\n", linebuf);
}

void hexdump(void *ptr, int buflen)
{
  unsigned char *buf = (unsigned char*)ptr;
  int i, j;
  for (i=0; i<buflen; i+=16) {
    printf("%06x: ", i);
    for (j=0; j<16; j++)
      if (i+j < buflen)
        printf("%02X ", buf[i+j]);
      else
        printf("   ");
    printf(" ");
    for (j=0; j<16; j++)
      if (i+j < buflen)
        printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
    printf("\n");
  }
}

void dosbox_hexdump_file(int segment, void *ptr, int buflen)
{
  FILE *fp = fopen("memdump.txt", "w");

  unsigned char *buf = (unsigned char*)ptr;
  int i, j;
  for (i=0; i<buflen; i+=16) {
    fprintf(fp, "%04X:%04X  ", segment, i);
    for (j=0; j<16; j++)
      if (i+j < buflen)
        fprintf(fp, " %02X", buf[i+j]);
      else
        fprintf(fp, "   ");
#if 0
    for (j=0; j<16; j++)
      if (i+j < buflen)
        printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
#endif
    fprintf(fp, " \n");
  }

  fclose(fp);
}
