// avrtmon
// Packet-switched communication layer - Packet interface
// Test Unit
// Paolo Lucchesi - Tue 06 Aug 2019 01:55:04 PM CEST
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "packet.h"


void packet_print(const packet_t *packet); // In 'sources/packet.c'

enum PARAMS_VALIDITY_E { PARAMS_INVALID = 0, PARAMS_VALID = 1 };

static void test_params(enum PARAMS_VALIDITY_E params_are_valid,
    packet_type_t type, const uint8_t *data, uint8_t data_size, packet_t *dest) {
  printf("packet_craft(%x, %p, %d, %p)\n", type, data, data_size, dest);

  uint8_t ret = packet_craft(type, data, data_size, dest);
  uint8_t this_passed = params_are_valid ^ ret ? 1 : 0;
  printf("[%s] Packet was %screated\n",
      this_passed ? "PASSED" : "FAILED", ret == 0 ? "" : "not ");
  if (!this_passed)
    packet_print(dest);

  putchar('\n');
}


int main(int argc, const char *argv[]) {
  printf("avrtmon - Packet Interface Test Unit\n\n");
  packet_t _p, *p = &_p;

  // Print dimensions of data structures
  printf("sizeof(packet_t)        -> %d bytes\n", sizeof(_p));
  printf("sizeof(packet_header_t) -> %d bytes\n", sizeof(_p.header));
  printf("sizeof(crc_t)           -> %d bytes\n", sizeof(_p.crc));
  printf("sizeof(packet_t.data)   -> %d bytes\n", sizeof(_p.data));
  putchar('\n');

  // Mock data message to perform tests
  const char *data = "Ho tante noci di cocco splendide";
  const uint8_t data_size = strlen(data);
  assert(data_size < PACKET_DATA_MAX_SIZE);


  // Try to craft some malformed packets
  printf("\nTesting packet_craft() against malformed parameters\n\n");

  // Test against trivially inconsistent parameters
  test_params(PARAMS_INVALID, 0xFF, data, data_size, p);
  test_params(PARAMS_INVALID, PACKET_TYPE_DAT, NULL, 123, p);
  test_params(PARAMS_INVALID, PACKET_TYPE_DAT, data, PACKET_DATA_MAX_SIZE+1, p);
  test_params(PARAMS_INVALID, PACKET_TYPE_DAT, data, data_size, NULL);

  // ACK and ERR packets must not bring data with them
  test_params(PARAMS_INVALID, PACKET_TYPE_ACK, data, data_size, p);
  test_params(PARAMS_INVALID, PACKET_TYPE_ERR, data, data_size, p);

  // Test against good parameters
  printf("\nTesting packet_craft() against good parameters\n\n");
  test_params(PARAMS_VALID, PACKET_TYPE_DAT, data, data_size, p);

  return 0;
}
