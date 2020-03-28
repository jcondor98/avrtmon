// avrtmon
// Packet-switched communication layer - Packet interface
// Source file
// Paolo Lucchesi - Sat 03 Aug 2019 12:33:20 PM CEST
#include <string.h>
#include "packet.h"

#if defined(TEST) || !defined(AVR)
#include <stdio.h>
#include <ctype.h>
#endif

// Bit fields parameter for the downstainding base type
#define BITFIELD_BASE_TYPE uint8_t
#define BIT_NUM (8*sizeof(BITFIELD_BASE_TYPE))
#define ALL_SET ((BITFIELD_BASE_TYPE) ~0U)

// Expose some functions only when testing
#ifdef TEST
#define TEST_EXPOSED
#else
#define TEST_EXPOSED static inline
#endif


// Set the bits in the interval [a,b]
static inline BITFIELD_BASE_TYPE _bitmask(uint8_t a, uint8_t b) {                        
  return (ALL_SET >> (BIT_NUM-b-1)) & (ALL_SET << a);
} 

// Get a number identified by the bit interval [a,b] as an unsigned integer
static inline BITFIELD_BASE_TYPE _bitfield_get(uint8_t src, uint8_t a, uint8_t b) {
  return (src & _bitmask(a,b)) >> a;
}

// Set a number in the bit interval [a,b] (no side effects)
static inline BITFIELD_BASE_TYPE _bitfield_set(uint8_t src, uint8_t val, uint8_t a, uint8_t b) {
  const BITFIELD_BASE_TYPE bitmask = _bitmask(a,b);
  return (src & ~bitmask) ^ ((val << a) & bitmask);
}

// Setters (source at the bottom of this source file
TEST_EXPOSED void packet_set_type(packet_t *p, uint8_t type);
TEST_EXPOSED void packet_set_size(packet_t *p, uint8_t size);
TEST_EXPOSED void packet_set_id(packet_t *p, uint8_t id);



// [AUX] Attach the header parity bit to a packet
static inline void attach_header_parity(packet_t *p) {
  p->header[1] &= ~0x01;
  p->header[1] |= packet_header_parity(p);
}


// Craft a packet (which is preallocated as 'dest')
// Note that the passed data is sent 'as is', without caring about endianess
uint8_t packet_craft(uint8_t id, packet_type_t type, const uint8_t *data,
    uint8_t data_size, packet_t *dest) {

  // Check against malformed parameters
  if ((type <= 2 && (data || data_size)) || type > PACKET_TYPE_COUNT || !dest ||
      (!data && data_size) || data_size > PACKET_DATA_MAX_SIZE ||
      (type == PACKET_TYPE_HND && id != 0))
    return 1;

  // Initialize header
  packet_set_type(dest, type);
  packet_set_id(dest, id);
  packet_set_size(dest, PACKET_HEADER_SIZE + data_size + sizeof(crc_t));

  // Compute packet header parity
  attach_header_parity(dest);

  // Fill data buffer (without CRC)
  for (uint8_t i=0; i < data_size; ++i)
    dest->data[i] = data[i];

  // Attach the CRC
  crc_t *crc_p = (crc_t*)(dest->data + data_size);
  (*crc_p) = crc(dest, packet_get_size(dest) - sizeof(crc_t));

  return 0;
}


// Same as above, but takes a packet instead of an id
uint8_t packet_ack(const packet_t *p, packet_t *dest) {
  return p ? packet_craft(packet_get_id(p), PACKET_TYPE_ACK, NULL, 0, dest) : 1;
}

// Send an error packet (relative to a packet_t structure)
uint8_t packet_err(const packet_t *p, packet_t *dest) {
  return p ? packet_craft(packet_get_id(p), PACKET_TYPE_ERR, NULL, 0, dest) : 1;
}


// Check an entire packet via CRC 
// Returns 0 if the packet is sane, 1 if it is corrupted
// It is assumed that the header is sane
uint8_t packet_check_crc(const packet_t *p) {
  return (!p || crc_check(p, packet_get_size(p)) != 0) ? 1 : 0;
}

// Check the sanity of a packet header
// Returns 0 if the packet is sane, 1 otherwise
uint8_t packet_check_header(const packet_t *p) {
  if (p && packet_get_size(p) >= PACKET_MIN_SIZE && (packet_brings_data(p) ||
        packet_get_size(p) == PACKET_MIN_SIZE) && packet_header_parity(p) == 0)
    return 0;
  else return 1;
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
  return packet_get_type(p) > 2 ? 1 : 0;
}


// Getters for packet header fields
uint8_t packet_get_type(const packet_t *p) {
  return p ? _bitfield_get(p->header[0], 4, 7) : 0;
}
uint8_t packet_get_id(const packet_t *p) {
  return p ? _bitfield_get(p->header[0], 0, 3) : 0;
}
uint8_t packet_get_size(const packet_t *p) {
  return p ? _bitfield_get(p->header[1], 1, 7) : 0;
}
uint8_t packet_get_header_par(const packet_t *p) {
  return p ? _bitfield_get(p->header[1], 0, 0) : 0;
}

// Compute the data size of a packet
uint8_t packet_data_size(const packet_t *p) {
  return p ? packet_get_size(p) - PACKET_HEADER_SIZE - sizeof(crc_t) : 0;
}


// Setters for packet header fields
TEST_EXPOSED void packet_set_type(packet_t *p, uint8_t type) {
  p->header[0] = _bitfield_set(p->header[0], type, 4, 7);
}
TEST_EXPOSED void packet_set_id(packet_t *p, uint8_t id) {
  p->header[0] = _bitfield_set(p->header[0], id, 0, 3);
}
TEST_EXPOSED void packet_set_size(packet_t *p, uint8_t size) {
  p->header[1] = _bitfield_set(p->header[1], size, 1, 7);
}


// Print out a complete representation of a packet
#if defined(TEST) || !defined(AVR)
void packet_print(const packet_t *p) {
  if (!p) {
    printf("Pointer to packet is NULL\n");
    return;
  }

  const uint8_t id = packet_get_id(p);
  const uint8_t size = packet_get_size(p);
  const uint8_t type = packet_get_type(p);

  static const char type_str[PACKET_TYPE_COUNT][4] = {
    "HND", "ACK", "ERR", "CMD", "CTR", "DAT" };
  printf("\nPrinting packet\n"
         "Type: %s\n"
         "ID  : %d\n"
         "Size: %d\n",
      type >= PACKET_TYPE_COUNT ? "UNKNOWN" : type_str[type], id, size);

  if (packet_brings_data(p)) {
    printf("Data (hex):");
    for (uint8_t i=0; i < size - 1; ++i) {
      if (isprint(p->data[i]))
        putchar(p->data[i]);
      else printf(" %hhx", p->data[i]);
    }
    putchar('\n');
  }

  printf("CRC: %hhx\n", ((uint8_t*) p)[size - 1]);
  printf("Raw Data (hex):");
  for (uint8_t i=0; i < size; ++i)
    printf(" %2hhx", ((uint8_t*) p)[i]);
  putchar('\n');
  putchar('\n');
}
#endif
