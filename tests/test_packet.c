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


// Parameterized test of 'packet_craft()'
static void test_params(packet_validity_t params_are_valid, packet_type_t type,
    const void *data, uint8_t data_size, packet_t *dest);

// Test packet integrity
static void test_packet_integrity(packet_validity_t should_be_valid,
    const packet_t *packet);


int main(int argc, const char *argv[]) {
  printf("avrtmon - Packet Interface Test Unit\n\n");
  packet_t _p, *p = &_p;

  /* Debug info which has been used for the unit test itself
  // Print dimensions of data structures
  printf("sizeof packet        -> %d bytes\n", sizeof(_p));
  printf("sizeof packet header -> %d bytes\n", (void*)(&(p->data)) - (void*)p);
  printf("sizeof packet data   -> %d bytes\n", sizeof(_p.data) - sizeof(crc_t));
  printf("sizeof crc data type -> %d bytes\n", sizeof(crc_t));
  putchar('\n');
  printf("Address of packet      -> %p\n", p);
  printf("Address of packet data -> %p\n", &(p->data));
  putchar('\n');
  */

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


  // Test packet header computing algorithm
  memset(p, 0x00, PACKET_HEADER_SIZE);
  test_expr(packet_header_parity(p) == 0,
      "Packet header computing algorithm should work when bits are even");
  p->header_par = 1;
  test_expr(packet_header_parity(p) != 0,
      "Packet header computing algorithm should work when bits are odd");

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


// Test packet integrity
static void test_packet_integrity(packet_validity_t should_be_valid,
    const packet_t *packet) {

  uint8_t is_not_valid =
    packet_check_header(packet) != 0 || packet_check_crc(packet) != 0;
  int expr = should_be_valid ^ is_not_valid;

  test_expr(expr,
      "Packet should %sbe valid (crc_check returned %d)",
      is_not_valid ? "not " : "");

  if (!expr) packet_print(packet);
}
