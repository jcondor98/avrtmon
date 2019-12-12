// avrtmon
// Packet-switched communication layer - Head file
// Paolo Lucchesi - Sat 03 Aug 2019 12:33:20 PM CEST
#ifndef __PACKET_LAYER_H
#define __PACKET_LAYER_H
#include <stdint.h>
#include <stddef.h> // offsetof macro
#include "crc.h"

// Packet hardcoded properties and parameters
#define PACKET_ID_MAX_VAL (~(0xFF << 4))
#define PACKET_DATA_MAX_SIZE 28
#define PACKET_HEADER_SIZE 2

// Packet types
#define PACKET_TYPE_COUNT 6
typedef enum PACKET_TYPE_E {
  PACKET_TYPE_HND = 0x00, // Handshake
  PACKET_TYPE_ACK = 0x01, // Acknowledgement
  PACKET_TYPE_ERR = 0x02, // Communication error
  PACKET_TYPE_CMD = 0x03, // Command
  PACKET_TYPE_CTR = 0x04, // Control sequence (e.g. for commands)
  PACKET_TYPE_DAT = 0x05  // Data
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

// Check the packet header via parity bit
// Returns 0 if the header is sane, 1 if it is corrupted
uint8_t packet_check_header(const packet_t*);

// Compute the packet header parity bit
// Returns the even parity bit (1 if data bits sum is odd, 0 if even)
// Also works as a check, must return 0 (i.e. bits are even) if the packet
// header is not corrupted by a single bit flip
uint8_t packet_header_parity(const packet_t*);

// Check an entire packet via CRC 
// Returns 0 if the packet is sane, 1 if it is corrupted
uint8_t packet_check_crc(const packet_t*);

// Acknowledge a packet, passing the packet itself or its id
// Returns 0 on success, 1 otherwise
uint8_t packet_ack(const packet_t*, packet_t *dest);
uint8_t packet_ack_by_id(uint8_t id, packet_t *dest);

// Send an error packet (relative to a packet or a packet id)
// Returns 0 on success, 1 otherwise
uint8_t packet_err(const packet_t*, packet_t *dest);
uint8_t packet_err_by_id(uint8_t id, packet_t *dest);

// Return 1 if the packet can bring data, 0 otherwise (i.e. ACK/ERR/HND)
uint8_t packet_brings_data(const packet_t*);


// Print out a complete representation of a packet
#if defined(TEST) || !defined(AVR)
void packet_print(const packet_t*);
#endif

#endif    // __PACKET_LAYER_H
