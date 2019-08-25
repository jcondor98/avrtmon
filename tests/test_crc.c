// avrtmon
// Packet-switched communication layer - CRC Tests
// Paolo Lucchesi - Mon 05 Aug 2019 03:55:51 AM CEST
#include <stdio.h>
#include <string.h>
#include "test_framework.h"

#include "crc.h"


int main(int argc, const char *argv[]) {
  // Print informations about the CRC algorithm
  printf("avrtmon - CRC Unit Test\n\n"
         "CRC_NAME: %s\n"
         "CRC_POLY: 0x%2x\n"
         "CRC_INIT: 0x%2x\n\n",
         CRC_NAME, CRC_POLY, CRC_INIT);

  // Leave room for trailing CRC (needed later)
  char s[] = "123456789";
  size_t s_len = strlen(s);

  // Test CRC computation
  crc_t computed_crc = crc(s, s_len);
  test_expr(computed_crc == CRC_CHECK, "CRC should match the expected one");
  printf("String  : %s\nCRC     : 0x%x\nExpected: 0x%x\n\n",
         s, crc(s, s_len), CRC_CHECK);

  // Test error checking
  s[s_len] = CRC_CHECK;
  s[s_len+1] = '\0';
  crc_t crc_check_result = crc_check(s, s_len + 1);
  test_expr(crc_check_result == 0,
      "Error checking should work (crc_check returned %d)", crc_check_result);

  test_summary();
  return 0;
}
