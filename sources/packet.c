// avrtmon
// Packet-switched communication layer - Packet interface
// Source file
// Paolo Lucchesi - Sat 03 Aug 2019 12:33:20 PM CEST
#include "packet.h"

// Must not be changed until the packet of id 'incoming_id' is acknowledged
// TODO: I don't think it will be useful here, shall be modified by the ISR of
//   the AVR U(S)ART
//static uint8_t incoming_id = 0;

// Craft a packet (which is preallocated as 'dest')
// Note that the passed data is sent 'as is', without caring about endianess
uint8_t packet_craft(packet_type_t type, const uint8_t *data, uint8_t data_size,
                     packet_t *dest) {

  // Sanitize passed parameters
  if (type & 0xF0 != 0 || !dest || !data || data_size == 0 || data_size > PACKET_DATA_MAX_SIZE)
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
