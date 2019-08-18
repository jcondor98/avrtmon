// avrtmon
// Packet-switched communication layer - Head file
// Paolo Lucchesi - Sat 03 Aug 2019 12:33:20 PM CEST
#ifndef __PACKET_LAYER_H
#define __PACKET_LAYER_H
#include <stdint.h>
#include "crc.h"

// Packet hardcoded properties and parameters
// TODO: Choose a proper, sensible max size for the packet data
#define PACKET_ID_MAX_VAL (~(0xFF << 4))
#define PACKET_DATA_MAX_SIZE 24
#define PACKET_HEADER_SIZE 2

// Packet types
typedef enum PACKET_TYPE_E {
  PACKET_TYPE_DAT = 0x00,
  PACKET_TYPE_CMD = 0x01,
  PACKET_TYPE_ACK = 0x02,
  PACKET_TYPE_ERR = 0x03
} packet_type_t;

// Packet type definition
typedef struct _packet_s {
  packet_type_t type : 4;
  unsigned id        : 4;
  unsigned size      : 7;
  unsigned header_par: 1; // Even parity bit for the header
  uint8_t data[PACKET_DATA_MAX_SIZE + sizeof(crc_t)];  // Data + trailing CRC
} packet_t;


// Craft a packet (which is preallocated as 'dest')
// Returns 0 if the passed parameters are consistent, or 1 otherwise
uint8_t packet_craft(packet_type_t type, const uint8_t *data, uint8_t size,
                     packet_t *dest);

// Check the integrity of DAT and CMD packets
// Returns 0 if the packet is sane (i.e. parities and CRCs match)
uint8_t packet_check(const packet_t *packet);

// Acknowledge a packet, passing the packet itself or its id
// Return a byte representing the entire ACK packet to send, without side effect
uint8_t packet_ack(const packet_t *packet);
uint8_t packet_ack_by_id(uint8_t id);

// Send an error packet (relative to a packet or a packet id)
// Return a byte representing the entire ERR packet to send, without side effect
uint8_t packet_err(const packet_t *packet);
uint8_t packet_err_by_id(uint8_t id);

#endif    // __PACKET_LAYER_H
