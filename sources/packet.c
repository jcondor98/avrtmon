// avrtmon
// Packet-switched communication layer - Packet interface
// Source file
// Paolo Lucchesi - Sat 03 Aug 2019 12:33:20 PM CEST
#include "packet.h"

#if defined(TEST) || !defined(AVR)
#include <stdio.h>
#include <ctype.h>
#endif


// [AUX] Attach the header parity bit to a packet
static inline void attach_header_parity(packet_t *p) {
  //p->header_par = packet_header_parity(p) & ~(p->header_par);
  p->header_par = 0;
  p->header_par = packet_header_parity(p);
}


// Craft a packet (which is preallocated as 'dest')
// Note that the passed data is sent 'as is', without caring about endianess
uint8_t packet_craft(packet_type_t type, const uint8_t *data, uint8_t data_size,
                     packet_t *dest) {

  // Check against malformed parameters
  if (type == PACKET_TYPE_ACK || type == PACKET_TYPE_ERR ||
      type > PACKET_TYPE_COUNT || !dest || (!data && data_size != 0) ||
      data_size > PACKET_DATA_MAX_SIZE)
    return 1;

  // ID of the packet to be crafted
  static uint8_t id = 0;

  // Treat a handshake specially
  if (type == PACKET_TYPE_HND) {
    dest->type = PACKET_TYPE_HND;
    dest->id   = 0;
    dest->size = PACKET_HEADER_SIZE;
    attach_header_parity(dest);
    id = 1;
    return 0;
  }

  // Initialize header
  dest->type = type;
  dest->id   = id;
  dest->size = PACKET_HEADER_SIZE + data_size + sizeof(crc_t);
  id = (id + 1) % PACKET_ID_MAX_VAL;

  // Compute packet header parity
  attach_header_parity(dest);

  // Fill data buffer (without CRC)
  for (uint8_t i=0; i < data_size; ++i)
    dest->data[i] = data[i];

  // Attach the CRC
  crc_t *crc_p = (crc_t*)(dest->data + data_size);
  (*crc_p) = crc(dest, dest->size - sizeof(crc_t));

  return 0;
}


// Acknowledge a packet, passing the packet itself or its id
// Returns a single byte to send to the other communication end
uint8_t packet_ack_by_id(uint8_t id, packet_t *dest) {
  if (!dest) return 1;
  dest->type = PACKET_TYPE_ACK;
  dest->id = id;
  dest->size = 2;
  attach_header_parity(dest);
  return 0;
}

// Same as above, but takes a packet instead of an id
uint8_t packet_ack(const packet_t *packet, packet_t *dest) {
  return packet ? packet_ack_by_id(packet->id, dest) : 1;
}


// Send an error packet (relative to a packet id)
uint8_t packet_err_by_id(uint8_t id, packet_t *dest) {
  attach_header_parity(dest);
  if (!dest) return 1;
  dest->type = PACKET_TYPE_ERR;
  dest->id = id;
  dest->size = 2;
  attach_header_parity(dest);
  return 0;
}

// Send an error packet (relative to a packet_t structure)
uint8_t packet_err(const packet_t *packet, packet_t *dest) {
  return packet ? packet_err_by_id(packet->id, dest) : 1;
}


// Check an entire packet via CRC 
// Returns 0 if the packet is sane, 1 if it is corrupted
// It is assumed that the header is sane
uint8_t packet_check_crc(const packet_t *p) {
  if (!p) return 1;

  // By default, consider sane packets that do not have a CRC
  if (!packet_brings_data(p))
    return 0;
  return (crc_check(p, p->size) != 0) ? 1 : 0;
}

// Check the sanity of a packet header
// Returns 0 if the packet is sane, 1 otherwise
uint8_t packet_check_header(const packet_t *packet) {
  if (packet && packet->size >= 2 &&
      (packet_brings_data(packet) || packet->size == 2) &&
      packet_header_parity(packet) == 0)
    return 0;
  return 1;
}


// Compute the packet header parity bit
// Returns the even parity bit (1 if data bits sum is odd, 0 if even)
// Also works as a check, must return 0 (i.e. bits are even) if the packet
// header is not corrupted by a single bit flip
uint8_t packet_header_parity(const packet_t *packet) {
  // Count set bits in a nibble in constant time
  static const uint8_t nibble_bitcount_tab[] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4
  };

  if (!packet) return 0;
  const uint8_t *p = (const uint8_t*) packet;

  // Compute parity - loop unrolled
  // NOTE: Works until the size of the header is changed
#if PACKET_HEADER_SIZE != 2
#error "Packet header must have size == 2 for a loop unrolled parity check"
#endif
  uint8_t bits_set = 0;
  bits_set += nibble_bitcount_tab[p[0] & 0x0F];
  bits_set += nibble_bitcount_tab[p[0] >> 4  ];
  bits_set += nibble_bitcount_tab[p[1] & 0x0F];
  bits_set += nibble_bitcount_tab[p[1] >> 4  ];

  return bits_set % 2;
}


// Return 1 if the packet can bring data, 0 otherwise (i.e. ACK/ERR/HND)
uint8_t packet_brings_data(const packet_t *p) {
  return p->type > 2 ? 1 : 0;
}


// [AUX] Compute the exact size of a packet header
uint8_t packet_header_size(const packet_t *packet) {
  const void *p = (const void*) packet;
  const void *p_data = (const void*)(packet->data);
  return p_data - p;
}



// Print out a complete representation of a packet
#if defined(TEST) || !defined(AVR)
void packet_print(const packet_t *packet) {
  if (!packet) {
    printf("Pointer to packet is NULL\n");
    return;
  }

  static const char type_str[PACKET_TYPE_COUNT][4] = {
    "HND", "ACK", "ERR", "CMD", "CTR", "DAT" };
  printf("Printing packet\n"
         "Type: %s\n"
         "ID  : %d\n"
         "Size: %d\n",
      type_str[packet->type], packet->id, packet->size);

  if (!packet_brings_data(packet)) return;

  for (uint8_t i=0; i < packet->size - 1; ++i) {
    if (isprint(packet->data[i]))
      putchar(packet->data[i]);
    else printf(" 0x%hhx ", packet->data[i]);
  }

  printf("\nCRC: %hhx\n", packet->data[packet->size]);
}
#endif
