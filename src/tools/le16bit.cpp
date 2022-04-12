// Tool for extracting contents out of dragon.com
// and creating constant tables in C / C++ code.
//
// Yes, xxd can probably handle this with the right
// arguments...

#include <cstdio>
#include <cstdint>

void extract_b152(FILE *fp)
{
  uint16_t com_start = 0xB152;

  if (fseek(fp, com_start - 0x100, SEEK_SET) != 0) {
    printf("Can't seek to offset\n");
    return;
  }

  printf("static uint8_t b152[256] = {\n ");
  for (int i = 0; i < (256 / 8); i++)
  {
    for (int j = 0; j < 8; j++) {
      uint8_t dval = fgetc(fp);
      printf(" ");
      printf("0x%02X,", dval);
    }
    printf(" // 0x%04X-0x%04X\n ", com_start, com_start + 7);
    com_start += 8;
  }
  printf("\r};\n");
}

void extract_ba52(FILE *fp)
{
  uint16_t com_start = 0xBA52;

  if (fseek(fp, com_start - 0x100, SEEK_SET) != 0) {
    printf("Can't seek to offset\n");
    return;
  }

  printf("static uint16_t ba52_table[256] = {\n ");
  for (int i = 0; i < (256 / 8); i++)
  {
    for (int j = 0; j < 8; j++) {
      uint16_t dval = fgetc(fp);
      dval += fgetc(fp) << 8;
      printf(" ");
      printf("0x%04X,", dval);
    }
    printf(" // 0x%04X\n ", com_start);
    com_start += 0x10;
  }
  printf("\r};\n");
}

// Little endian 16 bit extractor
void extract_b652(FILE *fp)
{
  uint16_t com_start = 0xB652;

  if (fseek(fp, 0xB552, SEEK_SET) != 0) {
    printf("Can't seek to offset\n");
    return;
  }

  printf("static uint16_t or_table_B652[256] = {\n ");
  for (int i = 0; i < (256 / 8); i++)
  {
    for (int j = 0; j < 8; j++) {
      uint16_t dval = fgetc(fp);
      dval += fgetc(fp) << 8;
      printf(" ");
      printf("0x%04X,", dval);
    }
    printf(" // 0x%04X\n ", com_start);
    com_start += 0x10;
  }
  printf("\r};\n");
}



int main(void)
{
  FILE *fp = fopen("DRAGON.COM", "rb");

  if (fp == NULL) {
    printf("Cannot open DRAGON.COM\n");
    return -1;
  }

  extract_ba52(fp);
  //extract_b152(fp);
  //extract_b652(fp);

  fclose(fp);

  return 0;
}
