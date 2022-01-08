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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

#include <offsets.h>
#include <tables.h>
#include <ui.h>

static const size_t fb_width = 320;
static const size_t fb_height = 200;
static const int BYTES_PER_PIXEL = 3; // RGB
static const size_t fb_size = fb_width * fb_height * BYTES_PER_PIXEL; // RGB
static unsigned char *fb_mem;

struct palette {
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
};

static const struct palette dos_palette[] = {
  { 0x00, 0x00, 0x00, 0xFF }, /* BLACK */
  { 0x00, 0x00, 0xAA, 0xFF }, /* BLUE */
  { 0x00, 0xAA, 0x00, 0xFF }, /* GREEN */
  { 0x00, 0xAA, 0xAA, 0xFF }, /* CYAN */
  { 0xAA, 0x00, 0x00, 0xFF }, /* RED */
  { 0xAA, 0x00, 0xAA, 0xFF }, /* MAGENTA */
  { 0xAA, 0x55, 0x00, 0xFF }, /* BROWN */
  { 0xAA, 0xAA, 0xAA, 0xFF }, /* LIGHT GRAY */
  { 0x55, 0x55, 0x55, 0xFF }, /* DARK GRAY */
  { 0x55, 0x55, 0xFF, 0xFF }, /* LIGHT BLUE */
  { 0x55, 0xFF, 0x55, 0xFF }, /* LIGHT GREEN */
  { 0x55, 0xFF, 0xFF, 0xFF }, /* LIGHT CYAN */
  { 0xFF, 0x55, 0x55, 0xFF }, /* LIGHT RED */
  { 0xFF, 0x55, 0xFF, 0xFF }, /* LIGHT MAGENTA */
  { 0xFF, 0xFF, 0x55, 0xFF }, /* YELLOW */
  { 0xFF, 0xFF, 0xFF, 0xFF } /* WHITE */
};

static void write_data()
{
  int line_num = 8;
  int rows = 0x88;
  int cols = 0x50;

  const unsigned char *src = ui_get_viewport_mem();

  for (int y = 0; y < rows; y++) {
    size_t fb_off = (line_num * (fb_width * BYTES_PER_PIXEL)) +
      (16 * BYTES_PER_PIXEL); // indentation
    for (int x = 0; x < cols; x++) {
      uint8_t al = *src++;

      int hi, lo;

      /* Each nibble represents a color */
      /* for example 0x82 represents color 8 then color 2. */
      hi = (al >> 4) & 0xf;
      lo = al & 0xf;

      fb_mem[fb_off++] = dos_palette[hi].r;
      fb_mem[fb_off++] = dos_palette[hi].g;
      fb_mem[fb_off++] = dos_palette[hi].b;
      fb_mem[fb_off++] = dos_palette[lo].r;
      fb_mem[fb_off++] = dos_palette[lo].g;
      fb_mem[fb_off++] = dos_palette[lo].b;
    }
    line_num++;
  }
}

static void init_buffers()
{
  fb_mem = new unsigned char[fb_size];
  memset(fb_mem, 0, fb_size);
}

static unsigned char* read_file(const char *fname)
{
  FILE *fp = fopen(fname, "rb");
  if (fp == nullptr) {
    return nullptr;
  }

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  if (fsize == -1) {
    fclose(fp);
    return nullptr;
  }
  rewind(fp);

  auto *bytes = new unsigned char[fsize];
  auto br = fread(bytes, 1, fsize, fp);
  if (br != (unsigned long)fsize) {
    fclose(fp);
    delete[] bytes;
    return nullptr;
  }

  return bytes;
}


static void process_input(unsigned char *src, int offset)
{
  unsigned char *ds = src + offset;

  uint16_t ax;
  ax = *ds;
  ds++;
  ax += (*ds) << 8;
  printf("%s: BX: 0x%04X AX: 0x%04X\n", __func__, offset, ax);

  if (ax == 0)
    return;

  uint16_t word_104F = ax;
  // 0xCF8
  ds = src + word_104F;
  viewport_data vp{};
  vp.data = ds;
  vp.xpos = 0;
  vp.ypos = 0;
  sub_CF8(vp.data, &vp);
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(stderr, "Invalid argument, needs raw data file.\n");
    fprintf(stderr, "Example: ./ppmwrite res.bin\n");
    exit(1);
  }

  auto *src_bytes = read_file(argv[1]);
  if (src_bytes == nullptr) {
    fprintf(stderr, "Failed to load: %s\n", argv[1]);
    exit(1);
  }

  init_buffers();
  init_offsets(0x50);
  init_viewport_memory();

  process_input(src_bytes, 4);

  delete[] src_bytes;

  // Dump to PPM
  FILE *imageFile = fopen("viewport.ppm","wb");

  if (imageFile == nullptr){
    perror("ERROR: Cannot open output file");
    exit(EXIT_FAILURE);
  }

  fprintf(imageFile,"P6\n");               // P6 filetype
  fprintf(imageFile,"%zu %zu\n", fb_width, fb_height);   // dimensions
  fprintf(imageFile,"255\n");              // Max pixel

  write_data();

  fwrite(fb_mem, 1, fb_size, imageFile);
  fclose(imageFile);
}
