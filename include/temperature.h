// avrtmon
// Unified temperature interface - Head file
// Paolo Lucchesi - Wed 25 Mar 2020 12:18:47 AM CET
#ifndef __TEMPERATURE_INTERFACE_H
#define __TEMPERATURE_INTERFACE_H
#include <stdint.h>

// ID type for a temperature (also used as an index in the temperature db)
typedef uint16_t temperature_id_t;

// Temperature type definition
// In practice, represent the raw input coming from an LM35 sensor, but the
// data type could be changed in any moment to store additional informations
typedef uint16_t temperature_t;


// To safely share databases metadata, a data structure with a fixed, machine
// and compiler independent size is used. Do not try to access it directly in
// any way.
#define SIZEOF_TEMPERATURE_DB_INFO (sizeof(temperature_id_t) + sizeof(uint8_t))
typedef uint8_t temperature_db_info_t[SIZEOF_TEMPERATURE_DB_INFO];

// Pack a data stucture of fixed, machine-independent size containing info on
// a temperature database
uint8_t temperature_db_info_pack(temperature_db_info_t dest,
    uint8_t id, temperature_id_t count);

// Extract data from a packet database info structure
uint8_t temperature_db_info_extract(const temperature_db_info_t src, uint8_t *id,
    temperature_id_t *count);


// Import AVR/Host side specific interface
#ifndef __TEMPERATURE_INDEPENDENT
#include "temperature_specific.h"
#endif // __TEMPERATURE_INDEPENDENT

#endif    // __TEMPERATURE_INTERFACE_H
