// avrtmon
// Temperature database - Head file
// Paolo Lucchesi - Wed 21 Aug 2019 04:14:15 PM CEST
#ifndef __TEMPERATURE_DB_H
#define __TEMPERATURE_DB_H
#include <stdint.h>

#ifdef TEST
#include "nvm_mock.h"
#else
#include "nvm.h"
#endif

// ID type for a temperature (also used as an index in the temperature db)
typedef uint8_t id_t;

// Temperature type definition
// In practice, represent the raw input coming from an LM35 sensor, but the
// data type could be changed in any moment to store additional informations
typedef uint16_t temperature_t;

// Parameters for the entire database
#define TEMP_DB_OFFSET 0
#define TEMP_DB_CAPACITY ((NVM_SIZE - TEMP_DB_OFFSET) / sizeof(temperature_t))

// Should the database be cached in the volatile memory?
// This avoids performing a NVM read when getting temperatures
#define TEMP_DB_CACHE

// Temperature database type definition
typedef struct _temperature_db_s {
  id_t capacity;
  id_t used;
  temperature_t items[TEMP_DB_CAPACITY];
} temperature_db_t;


// Setup for using the temperature database
// Does nothing if DB caching is not enabled
void temperature_init(void);

// Register a new temperature
// Returns 0 on success, 1 otherwise (e.g. if there is no more space)
uint8_t temperature_register(uint16_t raw_val);

// Get a temperature by id
// Returns 0 if the temperature exists, 1 otherwise
uint8_t temperature_get(id_t, temperature_t *dest);

// Returns the number of temperatures actually present in the database
id_t temperature_count(void);

// Returns the capacity of the database (in registerable temperatures)
id_t temperature_capacity(void);

// Reset the database, deleting all the temperatures
void temperature_db_reset(void);

// Copy the entire database in a data structure provided by the user
// Returns 0 on success, 1 otherwise
uint8_t temperature_fetch_entire_db(temperature_db_t *dest_db);

#endif    // __TEMPERATURE_DB_H
