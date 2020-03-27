// avrtmon
// Unified temperature interface - Source file
// Paolo Lucchesi - Wed 25 Mar 2020 12:18:55 AM CET
// The data structures is barely like this:
// struct _temperature_db_info_s {
//   temperature_id_t count;
//   uint16_t reg_resolution;
//   uint16_t reg_interval;
//   uint8_t  id;
// } temperature_db_info_t;
#define __TEMPERATURE_INDEPENDENT // Do not load specific implementations
#include "temperature.h"
#undef __TEMPERATURE_INDEPENDENT // Better safe than sorry

#define VOID(p) ((void*) p)

#define ADDR_COUNT(info)\
  ((temperature_id_t*)VOID(info))

#define ADDR_REG_RESOLUTION(info)\
  ((uint16_t*)(VOID(info)+sizeof(temperature_id_t)))

#define ADDR_REG_INTERVAL(info)\
  ((uint16_t*)(VOID(info)+sizeof(temperature_id_t)+sizeof(uint16_t)))

#define ADDR_ID(info)\
  ((uint8_t*)(VOID(info)+sizeof(temperature_id_t)+2*sizeof(uint16_t)))


// Pack a data stucture of fixed, machine-independent size containing info on
// a temperature database
// Returns 0 on success, 1 if 'dest' is not valid
uint8_t temperature_db_info_pack(temperature_db_info_t dest, uint8_t id,
    temperature_id_t count, uint16_t reg_resolution, uint16_t reg_interval) {
  if (!dest) return 1;
  *ADDR_COUNT(dest) = count;
  *ADDR_REG_RESOLUTION(dest) = reg_resolution;
  *ADDR_REG_INTERVAL(dest) = reg_interval;
  *ADDR_ID(dest) = id;
  return 0;
}

// Extract data from a packet database info structure
// Returns 0 on success, 1 if some pointer is not valid
uint8_t temperature_db_info_extract(const temperature_db_info_t src, uint8_t *id,
    temperature_id_t *count, uint16_t *reg_resolution, uint16_t *reg_interval) {
  if (!src) return 1;
  if (count)
    *count = *ADDR_COUNT(src);
  if (reg_resolution)
    *reg_resolution = *ADDR_REG_RESOLUTION(src);
  if (reg_interval)
    *reg_interval = *ADDR_REG_INTERVAL(src);
  if (id)
    *id = *ADDR_ID(src);
  return 0;
}
