#include <cstdio>
#include <stdint.h>

int main(void)
{
  FILE *fp = fopen("DRAGON.COM", "rb");
  if (fp == NULL) {
    printf("Cannot open DRAGON.COM\n");
    return -1;
  }

  if (fseek(fp, 0x3860, SEEK_SET) != 0) {
    printf("Can't seek to offset\n");
    goto done;
  }

  for (int i = 0; i <= 0xFF; i++)
  {
    printf("static void op_%02X();\n", i);
  }

  for (int i = 0; i <= 0xFF; i++)
  {

    uint16_t addr = fgetc(fp);
    addr += fgetc(fp) << 8;

    printf("{ op_%02X, \"0x%04X\" },\n", i, addr);
  }

done:
  fclose(fp);

  return 0;
}
