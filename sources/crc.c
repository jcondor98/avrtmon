// avrtmon
// Packet-switched communication layer - Cyclic Redundancy Checksum
// Source file
// Paolo Lucchesi - Mon 05 Aug 2019 10:38:48 PM CEST
#include "crc.h"


// Execute one round of the division by the polynomial of the CRC algorithm
// TODO: Use a lookup table for XOR divisions
static inline crc_t crc_division_round(crc_t current_crc, uint8_t byte) {
  current_crc ^= byte;                               // Consider the given byte
  for (uint8_t k=0; k < 8; ++k) {                    // For each bit
    if (current_crc & 0x80)                          // Test if MSB is set
      current_crc = (current_crc << 1) ^ CRC_POLY;   // If yes, do the division
    else current_crc <<= 1;                          // Else, skip the iteration
  }
  return current_crc;
}

// Compute the CRC
// 'size' is a byte, since the CRC will be used for packets (no need for size_t)
crc_t crc(const void *data, uint8_t size) {
  if (!data) return (crc_t) 0;

  const uint8_t *_data = (const uint8_t*) data;
  register crc_t crc = CRC_INIT;

  for (uint8_t i=0; i < size; ++i)
    crc = crc_division_round(crc, _data[i]);

  return crc;
}


// Check a chunk of data with a trailing CRC
// Basically, it performs a division of the entire message by the polynomial.
// If there is no remainder, the message should not be corrupted (at least by
// a detectable error).
// Returns the remainder of the division, but it can be safely converted to a
// smaller data type if only knowing if the message is corrupted is relevant
crc_t crc_check(const void *data, uint8_t size) {
  if (!data) return ~((crc_t) 0);

  const uint8_t *_data = (const uint8_t*) data;
  crc_t div_result = 0x00;
  for (uint8_t i=0; i < size; ++i)
    div_result = crc_division_round(div_result, _data[i]);

  return div_result;
}
