// avrtmon
// Packet-switched communication layer - CRC Tests
// Paolo Lucchesi - Mon 05 Aug 2019 03:55:51 AM CEST
#include <stdio.h>
#include <string.h>
#include "crc.h"

int main(int argc, const char *argv[]) {
  // Print informations about the CRC algorithm
  printf("avrtmon - CRC Unit Test\n\n"
         "CRC_NAME: %s\n"
         "CRC_POLY: 0x%2x\n"
         "CRC_INIT: 0x%2x\n\n",
         CRC_NAME, CRC_POLY, CRC_INIT);

  // Leave room for trailing CRC (needed later)
  char s[12] = "123456789";
  size_t s_len = strlen(s);

  // Test CRC computation
  printf("String  : %s\nCRC     : 0x%x\nExpected: 0x%x\n\n",
         s, crc(s, s_len), CRC_CHECK);

  // Test error checking
  s[s_len] = CRC_CHECK;
  s[s_len+1] = '\0';
  printf("Error checking does %swork\n", crc_check(s, 10) ? "not " : "");

  return 0;
}
