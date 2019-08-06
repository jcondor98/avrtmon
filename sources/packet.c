// avrtmon
// Packet-switched communication layer - Packet interface
// Source file
// Paolo Lucchesi - Sat 03 Aug 2019 12:33:20 PM CEST
#include "packet.h"

#ifdef DEBUG
#include <stdio.h>
#endif

// Must not be changed until the packet of id 'incoming_id' is acknowledged
// TODO: I don't think it will be useful here, shall be modified by the ISR of
//   the AVR U(S)ART
//static uint8_t incoming_id = 0;

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

  dest->header.type = type;
  dest->header.id   = id;
  dest->header.size = data_size + sizeof(packet_header_t) + sizeof(crc_t);
  id = (id + 1) % PACKET_ID_MAX_VAL;

  for (uint8_t i=0; i < data_size; ++i)
    dest->data[i] = data[i];

  dest->crc = crc(dest, sizeof(dest) - sizeof(crc_t));

  return 0;
}


// Send a packet
void packet_send(const packet_t *packet) {
  // TODO: Implement packet_send
}


// [DEBUG] Print out a complete representation of a packet
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
      type_str[packet->header.type], packet->header.id, packet->header.size);

  for (uint8_t i=0; i < packet->header.size; ++i)
    putchar(packet->data[i]);
  putchar('\n');
  printf("CRC: %ld\n\n", ((long)packet->crc)); // Support up to CRC-64
}
