// avrtmon
// Unified temperature interface - Source file
// Paolo Lucchesi - Wed 25 Mar 2020 12:18:55 AM CET
#define __TEMPERATURE_INDEPENDENT // Do not load specific implementations
#include "temperature.h"
#undef __TEMPERATURE_INDEPENDENT // Better safe than sorry


// Pack a data stucture of fixed, machine-independent size containing info on
// a temperature database
// Returns 0 on success, 1 if 'dest' is not valid
uint8_t temperature_db_info_pack(temperature_db_info_t dest,
    uint8_t id, temperature_id_t count) {
  if (!dest) return 1;
  void *_dest = (void*) dest;
  *((temperature_id_t*) _dest) = count;
  *((uint8_t*)(_dest + sizeof(temperature_id_t))) = id;
  return 0;
}

// Extract data from a packet database info structure
// Returns 0 on success, 1 if some pointer is not valid
uint8_t temperature_db_info_extract(const temperature_db_info_t src, uint8_t *id,
    temperature_id_t *count) {
  if (!src || !id || !count) return 1;
  void *_src = (void*) src;
  *count = *((temperature_id_t*) _src);
  *id = *((uint8_t*)(_src + sizeof(temperature_id_t)));
  return 0;
}
