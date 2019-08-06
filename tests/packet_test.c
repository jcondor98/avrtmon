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

// Test a "Single-Byte Packet" (i.e. ACK or ERR)
static void test_sbp(const packet_t *packet, enum PACKET_TYPE_E type, uint8_t expected) {
  uint8_t (*sbp_by_id)(uint8_t)   = type == PACKET_TYPE_ACK ? packet_ack_by_id : packet_err_by_id;
  uint8_t (*sbp)(const packet_t*) = type == PACKET_TYPE_ACK ? packet_ack : packet_err;
  const char *type_str = type == PACKET_TYPE_ACK ? "ack" : "err";

  printf("Testing packet_%s_by_id\n", type_str);
  printf("%3s packet value: 0x%x\n", type_str, sbp_by_id(packet->header.id));
  printf("Expected value  : 0x%x\n\n", expected);

  printf("Testing packet_%s\n", type_str);
  printf("%3s packet value: 0x%x\n", type_str, sbp(packet));
  printf("Expected value  : 0x%x\n\n", expected);

  const char *str_format = "[%s] %s function and its by-id equivalent %s return the same value\n\n";
  if (sbp(packet) == sbp_by_id(packet->header.id))
    printf(str_format, "PASSED", type_str, "do");
  else
    printf(str_format, "FAILED", type_str, "do not");
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

  // Test acknowledgement and error packets
  // We assing a non-zero id to the packet which was previously crafted
  // Expected values were manually calculated
  _p.header.id = 0x03;
  test_sbp(p, PACKET_TYPE_ACK, (uint8_t) 0x23);
  test_sbp(p, PACKET_TYPE_ERR, (uint8_t) 0x33);

  return 0;
}
