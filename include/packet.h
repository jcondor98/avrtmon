// avrtmon
// Packet-switched communication layer - Head file
// Paolo Lucchesi - Sat 03 Aug 2019 12:33:20 PM CEST
#ifndef __PACKET_LAYER_H
#define __PACKET_LAYER_H
#include <stdint.h>
#include "crc.h"

// Packet hardcoded properties and parameters
#define PACKET_ID_MAX_VAL (~(0xFF << 4))
#define PACKET_DATA_MAX_SIZE 48

// Packet types
typedef enum PACKET_TYPE_E {
  PACKET_TYPE_DAT = 0x00,
  PACKET_TYPE_CMD = 0x01,
  PACKET_TYPE_ACK = 0x02,
  PACKET_TYPE_ERR = 0x03
} packet_type_t;

// Packet header type definition
// TODO: Add a CRC (likely parity bit) to the header
typedef struct _packet_header_s {
  packet_type_t type : 4;
  unsigned id : 4;
  uint8_t size;
} packet_header_t;

// Packet type definition
typedef struct _packet_s {
  packet_header_t header;
  uint8_t data[PACKET_DATA_MAX_SIZE];
  crc_t crc;
} packet_t;


// Craft a packet (which is preallocated as 'dest')
// Returns 0 if the packet is well formed (i.e. passed parameters are good), or
// 1 otherwise
uint8_t packet_craft(packet_type_t type, const uint8_t *data, uint8_t size,
                     packet_t *dest);

// Acknowledge a packet, passing the packet itself or its id
// Return a byte representing the entire ACK packet to send
uint8_t packet_ack(const packet_t *packet);
uint8_t packet_ack_by_id(uint8_t id);

// Send an error packet (relative to a packet id)
// Return a byte representing the entire ERR packet to send
uint8_t packet_err(const packet_t *packet);
uint8_t packet_err_by_id(uint8_t id);

#endif    // __PACKET_LAYER_H
