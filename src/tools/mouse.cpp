//
// Copyright (c) 2023 Devin Smith <devin@devinsmith.net>
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

// A simple program to extract mouse cursors from dragon.com

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <tables.h>

struct mouse_cursor {
  const char *name;
  uint16_t offset;
  int width;
  int height;
  unsigned char *data;
};

static struct mouse_cursor cursors[6] = {
  { "Pointer with finger", 0x647C },
  { "Hand Stop/Wait cursor", 0x64EE },
  { "Forward",0x6654, },
  { "Turn around", 0x65D2, },
  { "Turn right", 0x66C6, },
  { "Turn left", 0x6550 }
};

static constexpr size_t num_cursors = sizeof(cursors) / sizeof(cursors[0]);

static const int fb_width = 320;
static const int fb_height = 200;
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

static void init_buffers()
{
  fb_mem = new unsigned char[fb_size];
  memset(fb_mem, 0, fb_size);
}

// 0x2463
static void plot_pixel(uint8_t line_num, int color, int x_off)
{
  size_t fb_off = get_line_offset(line_num);
  fb_off *= BYTES_PER_PIXEL;
  fb_off += (x_off * BYTES_PER_PIXEL);

  fb_mem[fb_off++] = dos_palette[color].r;
  fb_mem[fb_off++] = dos_palette[color].g;
  fb_mem[fb_off++] = dos_palette[color].b;
}

 void sub_23A1(const struct mouse_cursor *cursor, int x_pos, int y_pos)
{
  int dx;
  uint8_t line_pos;
  const uint8_t *ds = cursor->data;

  line_pos = (uint8_t)y_pos;

  // 241E
  for (int i = 0; i < cursor->height; i++) {
    dx = x_pos;
    // 0x2426
    for (int j = 0; j < cursor->width; j++) {
      uint8_t pixel_byte = *ds++;
      if (dx >= fb_width) {
        // 245E
        ds += (cursor->width - j);
        break;
      }

      // Extract nibble (4 bits) out of byte
      // Most significant nibble
      uint8_t msn = pixel_byte;
      msn = msn >> 4;
      msn = msn & 0xF;
      if (msn != 6) { // Don't plot brown pixels?
        plot_pixel(line_pos, msn, dx);
      }

      dx++;
      if (dx >= fb_width) {
        // 245E
        ds += (cursor->width - j);
        break;
      }

      // Least significant nibble
      pixel_byte = pixel_byte & 0x0F;
      if (pixel_byte != 6) {
        plot_pixel(line_pos, pixel_byte, dx);
      }
      dx++;
    }
    line_pos++;
  }
}

static bool load_cursors()
{
  FILE *dragonfp = fopen("dragon.com", "rb");
  if (dragonfp == nullptr) {
    perror("Failed to load dragon.com");
    return false;
  }

  for (size_t i = 0; i < num_cursors; i++) {
    long off = cursors[i].offset - 0x100; // COM START

    fseek(dragonfp, off, SEEK_SET);

    cursors[i].width = fgetc(dragonfp);
    cursors[i].height = fgetc(dragonfp);

    size_t data_sz = cursors[i].width * cursors[i].height;
    cursors[i].data = new unsigned char[data_sz];

    if (fread(cursors[i].data, 1, data_sz, dragonfp) != data_sz) {
      fprintf(stderr, "Failed to read %zu bytes from dragon.com file.\n", data_sz);
      delete [] cursors[i].data;
      return false;
    }
  }

  fclose(dragonfp);

  return true;
}

int main()
{
  if (!load_cursors())
    return -1;

  init_buffers();

  int x_pos = 10;

  for (size_t i = 0; i < num_cursors; i++) {
    printf("Drawing mouse cursor %zu\n", i);

    sub_23A1(&cursors[i], x_pos, 10);

    x_pos += 20;
  }

  // Dump to PPM
  FILE *imageFile = fopen("mouse.ppm","wb");

  if (imageFile == nullptr){
    perror("ERROR: Cannot open output file");
    exit(EXIT_FAILURE);
  }

  fprintf(imageFile,"P6\n");               // P6 filetype
  fprintf(imageFile,"%d %d\n", fb_width, fb_height);   // dimensions
  fprintf(imageFile,"255\n");              // Max pixel

  fwrite(fb_mem, 1, fb_size, imageFile);
  fclose(imageFile);

  delete[] fb_mem;
  return 0;
}
