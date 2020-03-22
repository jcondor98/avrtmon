// avrtmon
// Packet-switched communication layer - Head file
// Paolo Lucchesi - Sat 03 Aug 2019 12:33:20 PM CEST
#ifndef __PACKET_LAYER_H
#define __PACKET_LAYER_H
#include <stdint.h>
#include <stddef.h> // offsetof macro
#include "crc.h"

// Packet hardcoded properties and parameters
#define PACKET_ID_WIDTH_BIT 4
#define PACKET_ID_MAX_VAL (1 << PACKET_ID_WIDTH_BIT)
#define PACKET_DATA_MAX_SIZE 28
#define PACKET_HEADER_SIZE 2
#define PACKET_MIN_SIZE (PACKET_HEADER_SIZE + sizeof(crc_t))
#define PACKET_MAX_SIZE sizeof(packet_t)

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
typedef uint16_t packet_header_t;
typedef struct _packet_s {
  uint8_t header[PACKET_HEADER_SIZE];
    //unsigned type      : 4;
    //unsigned id        : 4;
    //unsigned size      : 7;
    //unsigned header_par: 1; // Even parity bit for the header
  uint8_t data[PACKET_DATA_MAX_SIZE + sizeof(crc_t)];  // Data + trailing CRC
} packet_t;


// Craft a packet (which is preallocated as 'dest')
// Note that the passed data is sent 'as is', without caring about endianess
// Returns 0 if the passed parameters are consistent, or 1 otherwise
uint8_t packet_craft(uint8_t id, packet_type_t type, const uint8_t *data,
    uint8_t data_size, packet_t *dest);

// Compute the next or previous packet ID
#define packet_next_id(id) (((id) + 1) % PACKET_ID_MAX_VAL)
#define packet_prev_id(id) (((id) + PACKET_ID_MAX_VAL - 1) % PACKET_ID_MAX_VAL)

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
#define packet_ack_by_id(id,dest) packet_craft(id,PACKET_TYPE_ACK,NULL,0,dest)

// Send an error packet (relative to a packet or a packet id)
// Returns 0 on success, 1 otherwise
uint8_t packet_err(const packet_t*, packet_t *dest);
#define packet_err_by_id(id,dest) packet_craft(id,PACKET_TYPE_ERR,NULL,0,dest)

// Return 1 if the packet can bring data, 0 otherwise (i.e. ACK/ERR/HND)
uint8_t packet_brings_data(const packet_t*);

// Getters for packet header fields
uint8_t packet_get_header_par(const packet_t *p);
uint8_t packet_get_type(const packet_t *p);
uint8_t packet_get_size(const packet_t *p);
uint8_t packet_get_id(const packet_t *p);

// Print out a complete representation of a packet
#if defined(TEST) || !defined(AVR)
void packet_print(const packet_t*);
#endif

#endif    // __PACKET_LAYER_H
