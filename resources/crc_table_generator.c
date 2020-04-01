#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/crc.h"

#define ONE_BYTE_STR_MAX_SPACE 6
#define TAB "  "

static inline size_t print_byte(unsigned char byte, int space, int comma) {
  char fmt[16];
  unsigned char i = 0;

  if (space) fmt[i++] = ' ';

  const char *fmt_byte = byte & 0xF0 ? "0x%2hhx" : "0x0%1hhx";
  unsigned char fmt_byte_len = strlen(fmt_byte);
  memcpy(fmt + i, fmt_byte, fmt_byte_len);
  i += fmt_byte_len;

  if (comma) fmt[i++] = ',';
  fmt[i] = '\0';

  return printf(fmt, byte);
}

static inline size_t byte_str_space(unsigned char byte, int space, int comma) {
  return (space ? 1 : 0) + (comma ? 1 : 0) + 4;
}


int main(int argc, const char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <table-name>\n", argv[0]);
    return 1;
  }

  // Compute CRC for each byte of data
  static unsigned char crc_table[0x0100];
  for (unsigned short i=0; i <= 0xFF; ++i) {
    unsigned char byte = (unsigned char) i;
    crc_table[i] = crc(&byte, 1);
  }

  // Print the table C-friendly
  printf("\nstatic const uint8_t %s[] = {\n", argv[1]);
  for (unsigned short i=0; i <= 0xFF; ) {
    unsigned char line = 0;
    line += printf("%s", TAB);
    line += print_byte(crc_table[i++], 0, (i == 0xFF) ? 0 : 1);
    while (i <= 0xFF && line < 80 - byte_str_space(crc_table[i], 1, (i == 0xFF) ? 0 : 1))
      line += print_byte(crc_table[i++], 1, (i == 0xFF) ? 0 : 1);
    if (i <= 0xFF) putchar('\n');
  }

  printf("\n};\n\n");
  return 0;
}
