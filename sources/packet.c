// avrtmon
// Packet-switched communication layer - Packet interface
// Source file
// Paolo Lucchesi - Sat 03 Aug 2019 12:33:20 PM CEST
#include "packet.h"

#ifdef DEBUG
#include <stdio.h>
#endif

// [AUX] Check the packet header
uint8_t packet_header_parity(const packet_t *packet);


// Craft a packet (which is preallocated as 'dest')
// Note that the passed data is sent 'as is', without caring about endianess
uint8_t packet_craft(packet_type_t type, const uint8_t *data, uint8_t data_size,
                     packet_t *dest) {

  // Sanitize passed parameters
  if ((type != PACKET_TYPE_DAT && type != PACKET_TYPE_CMD) || !dest || !data ||
      data_size == 0 || data_size > PACKET_DATA_MAX_SIZE)
    return 1;

  // ID of the packet to be crafted
  static uint8_t id = 0;

  dest->type = type;
  dest->id   = id;
  dest->size = sizeof(packet_t) - PACKET_DATA_MAX_SIZE + data_size;
  id = (id + 1) % PACKET_ID_MAX_VAL;

  for (uint8_t i=0; i < data_size; ++i)
    dest->data[i] = data[i];

  // Make the CRC assignment independent of the CRC size
  crc_t *crc_p = (crc_t*)(dest->data + data_size);
  (*crc_p) = crc(dest, dest->size - sizeof(crc_t));

  return 0;
}


// Acknowledge a packet, passing the packet itself or its id
// Returns a single byte to send to the other communication end
uint8_t packet_ack_by_id(uint8_t id) {
  return ((id & 0xFC) != 0) ? 0 : (PACKET_TYPE_ACK << 4) | id;
}

// Same as above, but takes a packet instead of an id
uint8_t packet_ack(const packet_t *packet) {
  return packet ? packet_ack_by_id(packet->id) : 0;
}


// Send an error packet (relative to a packet id)
uint8_t packet_err_by_id(uint8_t id) {
  return ((id & 0xFC) != 0) ? 0 : (PACKET_TYPE_ERR << 4) | id;
}

// Send an error packet (relative to a packet_t structure)
uint8_t packet_err(const packet_t *packet) {
  return packet ? packet_err_by_id(packet->id) : 0;
}


// Check the integrity of DAT and CMD packets
// Returns 0 if the packet is sane (i.e. parities and CRCs match)
uint8_t packet_check(const packet_t *packet) {
  if (!packet) return 1;

  // Check parity for the packet header
  if (packet_header_parity(packet) != 0) return 1;

  // Check CRC
  if (crc_check(packet, packet->size) != 0) return 1;

  // After the checks above, we assume that the packet is sane
  return 0;
}



// [AUX] Compute the packet header parity bit
// Returns the parity bit (1 or 0)
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
  uint8_t bits_set = 0;
  bits_set += nibble_bitcount_tab[p[0] & 0x0F];
  bits_set += nibble_bitcount_tab[p[0] >> 4  ];
  bits_set += nibble_bitcount_tab[p[1] & 0x0F];
  bits_set += nibble_bitcount_tab[p[1] >> 4  ];

  return bits_set % 2 == 0 ? 0 : 1;
}



// [DEBUG] Print out a complete representation of a packet
#ifdef DEBUG
void packet_print(const packet_t *packet) {
  if (!packet) {
    printf("Pointer to packet is NULL\n");
    return;
  }

  static const char type_str[][4] = { "DAT", "CMD", "ACK", "ERR" };
  printf("Printing packet\n"
         "Type: %s\n"
         "ID  : %d\n"
         "Size: %d\n",
      type_str[packet->type], packet->id, packet->size);

  for (uint8_t i=0; i < packet->size; ++i)
    putchar(packet->data[i]);
  putchar('\n');
  // TODO: Print out CRC
}
#endif
