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

// Dragon Wars resource extractor.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>

#include "../lib/resource.h"

const char *_pname = "resextract";

static void usage(int err)
{
  if (err)
    fprintf(stderr, "Invalid argument\n");

  fprintf(stderr, "usage: %s (-i index) "
      "[[--help] | [-h]]\n", _pname);
  fprintf(stderr, "\t-i (index)       - Extract this index.\n");
  fprintf(stderr, "\t-h | --help      - Show this message\n");
  fprintf(stderr, "\t-d data_file     - Use this data file (instead of data1).\n");
  fprintf(stderr, "\t-o output.bin    - Output binary.\n");
  exit(err);
}

static void args_required(char *arg, const char *argname)
{
  fprintf(stderr, "%s: option '%s' expects a parameter (%s)\n", _pname, arg, 
      argname);
  exit(1);
}

static void dump(const struct resource *r, const char *out_file)
{
  FILE *fp = fopen(out_file, "wb");
  if (fp == nullptr) {
    perror("Failed to open dump file");
    return;
  }

  fwrite(r->bytes, 1, r->len, fp);

  fclose(fp);
}

int main(int argc, char *argv[])
{
  int index = -1;
  const char *data = "data1";
  const char *output = "output.bin";

  // quick and dirty argument parser
  while (--argc) {
    char *p = *++argv;

    if (!strcmp(p, "-i")) {
      if (!argv[1]) {
        args_required(p, "index");
      }
      index = atoi(*++argv), --argc;
    } else if (!strcmp(p, "-d")) {
      if (!argv[1]) {
        args_required(p, "data");
      }
      data = *++argv, --argc;
    } else if (!strcmp(p, "-o")) {
      if (!argv[1]) {
        args_required(p, "output");
      }
      output = *++argv, --argc;
    } else if (!strcmp(p, "--help")) {
      usage(0);
    } else if (!strcmp(p, "-h")) {
      usage(0);
    } else {
      usage(1);
    }
  }

  if (index == -1) {
    usage(1);
  }

  if (rm_init() == 0) {
    const struct resource *res = resource_load((resource_section)index);

    if (res != nullptr) {
      dump(res, output);
    }
  }
  rm_exit();

  return 0;
}
