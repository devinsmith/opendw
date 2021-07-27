#include <cstdio>
#include <cstdint>

// Little endian 16 bit extractor
//
// I couldn't figure out how to do this with xxd :(
int main(void)
{
  FILE *fp = fopen("DRAGON.COM", "rb");
  uint16_t com_start = 0xB652;

  if (fp == NULL) {
    printf("Cannot open DRAGON.COM\n");
    return -1;
  }

  if (fseek(fp, 0xB552, SEEK_SET) != 0) {
    printf("Can't seek to offset\n");
    goto done;
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

done:
  fclose(fp);

  return 0;
}
