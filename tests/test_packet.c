// avrtmon
// Packet-switched communication layer - Packet interface
// Test Unit
// Paolo Lucchesi - Tue 06 Aug 2019 01:55:04 PM CEST
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "include/test_framework.h"

#include "packet.h"

// Expected validity of a packet in a test
typedef enum PACKET_VALIDITY_E {
  PACKET_INVALID = 0, PACKET_VALID = 1
} packet_validity_t;


// Inspect a packet - Source code is located in 'sources/packet.c'
void packet_print(const packet_t *packet);

// Parameterized test of 'packet_craft()'
static void test_params(packet_validity_t params_are_valid, packet_type_t type,
    const void *data, uint8_t data_size, packet_t *dest);

// Test a "Single-Byte Packet" (i.e. ACK or ERR)
static void test_sbp(const packet_t *packet,
    packet_type_t type, uint8_t expected);

// Test packet integrity
static void test_packet_integrity(packet_validity_t should_be_valid,
    const packet_t *packet);


int main(int argc, const char *argv[]) {
  printf("avrtmon - Packet Interface Test Unit\n\n");
  packet_t _p, *p = &_p;
  //uint8_t p_raw = (uint8_t*) p;

  // Print dimensions of data structures
  printf("sizeof packet        -> %d bytes\n", sizeof(_p));
  printf("sizeof packet header -> %d bytes\n", (void*)(&(p->data)) - (void*)p);
  printf("sizeof packet data   -> %d bytes\n", sizeof(_p.data) - sizeof(crc_t));
  printf("sizeof crc data type -> %d bytes\n", sizeof(crc_t));
  putchar('\n');
  printf("Address of packet      -> %p\n", p);
  printf("Address of packet data -> %p\n", &(p->data));
  putchar('\n');

  // Mock data message to perform tests
  const char *data = "Ho tante noci";
  const uint8_t data_size = strlen(data);
  assert(data_size < PACKET_DATA_MAX_SIZE);


  // Try to craft some malformed packets
  printf("\nTesting packet_craft() against malformed parameters\n");

  // Test against trivially inconsistent parameters
  test_params(PACKET_INVALID, 0xFF, data, data_size, p);
  test_params(PACKET_INVALID, PACKET_TYPE_DAT, NULL, 123, p);
  test_params(PACKET_INVALID, PACKET_TYPE_DAT, data, PACKET_DATA_MAX_SIZE+1, p);
  test_params(PACKET_INVALID, PACKET_TYPE_DAT, data, data_size, NULL);

  // ACK and ERR packets must not bring data with them
  test_params(PACKET_INVALID, PACKET_TYPE_ACK, data, data_size, p);
  test_params(PACKET_INVALID, PACKET_TYPE_ERR, data, data_size, p);

  // Test against good parameters
  printf("\nTesting packet_craft() against good parameters\n");
  test_params(PACKET_VALID, PACKET_TYPE_DAT, data, data_size, p);


  // Test acknowledgement and error packets
  // We assing a non-zero id to the packet which was previously crafted
  // Expected values were manually calculated
  printf("\nTesting Single Byte Packet functions\n");
  _p.id = 0x03;
  test_sbp(p, PACKET_TYPE_ACK, (uint8_t) 0x23);
  test_sbp(p, PACKET_TYPE_ERR, (uint8_t) 0x33);


  // Test packet integrity checking
  packet_craft(PACKET_TYPE_DAT, (void*) data, data_size, p);

  printf("\nTesting packet_check() with corrupted header parity bit\n");
  p->header_par ^= 1;
  test_packet_integrity(PACKET_INVALID, p);
  p->header_par ^= 1;  // Revert previous change - 'p' is now valid again

  printf("\nTesting packet_check() with corrupted CRC\n");
  p->data[data_size] += 1;
  test_packet_integrity(PACKET_INVALID, p);
  p->data[data_size] -= 1;  // Revert previous change

  printf("\nTesting packet_check() with corrupted header\n");
  p->type = PACKET_TYPE_CMD;
  test_packet_integrity(PACKET_INVALID, p);
  p->type = PACKET_TYPE_DAT;  // Revert previous change

  printf("\nTesting packet_check() with corrupted data\n");
  p->data[0] += 1;
  test_packet_integrity(PACKET_INVALID, p);
  p->data[0] -= 1;  // Revert previous change

  printf("\nTesting packet_check() with sane package\n");
  test_packet_integrity(PACKET_VALID, p);


  test_summary();
  return 0;
}



// Auxiliary functions

// Test for 'packet_craft' with dynamic parameters
static void test_params(packet_validity_t params_are_valid, packet_type_t type,
    const void *data, uint8_t data_size, packet_t *dest) {

  uint8_t ret = packet_craft(type, data, data_size, dest);
  int expr = params_are_valid ^ ret;
  test_expr(expr,
      "packet_craft(%x, %p, %d, %p) should %screate a packet",
      type, data, data_size, dest, params_are_valid ? "" : "not ");

  if (!expr) // Dump packet info if the test fails
    packet_print(dest);
}


// Test a "Single-Byte Packet" (i.e. ACK or ERR)
static void test_sbp(const packet_t *packet, packet_type_t type,
    uint8_t expected) {

  uint8_t (*sbp_by_id)(uint8_t) =
    type == PACKET_TYPE_ACK ? packet_ack_by_id : packet_err_by_id;

  uint8_t (*sbp)(const packet_t*) =
    type == PACKET_TYPE_ACK ? packet_ack : packet_err;

  const char *type_str = type == PACKET_TYPE_ACK ? "ack" : "err";
  uint8_t computed_sbp = sbp(packet);
  uint8_t computed_sbp_by_id = sbp_by_id(packet->id);

  test_expr(computed_sbp_by_id,
    "packet_%s_by_id returned value should match the expected one", type_str);
  printf("%3s packet value: 0x%x\n", type_str, computed_sbp_by_id);
  printf("Expected value  : 0x%x\n", expected);

  test_expr(computed_sbp,
    "packet_%s returned value should match the expected one", type_str);
  printf("%3s packet value: 0x%x\n", type_str, computed_sbp);
  printf("Expected value  : 0x%x\n", expected);

  int expr = computed_sbp == computed_sbp_by_id;
  test_expr(expr,
      "%s function and its by-id equivalent %s return the same value",
      type_str, expr ? "do" : "do not");

  putchar('\n');
}

// Test packet integrity
static void test_packet_integrity(packet_validity_t should_be_valid,
    const packet_t *packet) {

  uint8_t is_not_valid = packet_check(packet);
  int expr = should_be_valid ^ is_not_valid;

  test_expr(expr,
      "Packet should %sbe valid (crc_check returned %d)",
      is_not_valid ? "not " : "");

  if (!expr) packet_print(packet);
}
