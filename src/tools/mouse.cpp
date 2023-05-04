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

static void init_buffers()
{
  fb_mem = new unsigned char[fb_size];
  memset(fb_mem, 0, fb_size);
}

static void sub_2463(int line_num, int color, int x_off)
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
  uint16_t ax, cx, dx;
  int line_pos;
  uint16_t save_cx, save_ax;
  const uint8_t *ds = cursor->data;

  ax = cursor->width;
  cx = cursor->height;
  cx = cx & 0xff;
  line_pos = y_pos;

  // 241E
  do {
    save_cx = cx;
    save_ax = ax;
    cx = ax;
    dx = x_pos;
    // 0x2426
    do {
      ax = *ds++;
      if (dx >= 0x140) {
        // 245E
        cx--;
        ds += cx;
        break;
      }
      uint16_t tmp_ax = ax;
      ax = ax >> 4;
      ax = ax & 0xF;
      if (ax != 6) { // Don't plot brown pixels?
        sub_2463(line_pos, ax, dx);
      }
      ax = tmp_ax;
      dx++;
      if (dx >= 0x140) {
        // 245E
        cx--;
        ds += cx;
        break;
      }
      ax = ax & 0x0F;
      if (ax != 6) {
        sub_2463(line_pos, ax, dx);
      }
      dx++;
    } while (--cx > 0);
    line_pos++;
    ax = save_ax;
    cx = save_cx;
  } while (--cx > 0);
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
  load_cursors();

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
  fprintf(imageFile,"%zu %zu\n", fb_width, fb_height);   // dimensions
  fprintf(imageFile,"255\n");              // Max pixel

  fwrite(fb_mem, 1, fb_size, imageFile);
  fclose(imageFile);

  delete[] fb_mem;
  return 0;
}
