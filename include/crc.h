// avrtmon
// Packet-switched communication layer - Head file
// Paolo Lucchesi - Sat 03 Aug 2019 12:33:20 PM CEST
#ifndef __CRC_H
#define __CRC_H
#include <stdint.h>

// CRC Algorithm parameters
// The standard CRC-8 is used, but any other algorithm which does not make use
// of data reflection or xorout is supported
#define CRC_NAME "CRC-8"
#define CRC_POLY 0x07
#define CRC_INIT 0x00
#define CRC_CHECK 0xF4

// CRC type definition
// Makes the packet type definition resilient to a change (in size) of the CRC
typedef uint8_t crc_t;

// Compute the CRC of a chunk of data
// Do not include trailing zeroes as a room for the CRC (if there are, then
// truncate the 'size' parameter so they are not counted)
crc_t crc(const void *data, uint8_t size);

// Check a chunk of data with a trailing CRC
// You have to count the bytes taken by the CRC in the 'size' parameter
// Basically, it performs a division of the entire message by the polynomial.
// If there is no remainder, the message should not be corrupted (at least by
// a detectable error).
// Returns the remainder of the division, but it can be safely converted to a
// smaller data type if only knowing if the message is corrupted is relevant
crc_t crc_check(const void *data, uint8_t size);

#endif    // __CRC_H
