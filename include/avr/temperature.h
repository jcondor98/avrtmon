// avrtmon
// Temperature database - Head file
// Paolo Lucchesi - Wed 21 Aug 2019 04:14:15 PM CEST
#ifndef __TEMPERATURE_DB_H
#define __TEMPERATURE_DB_H
#include <stdint.h>

// ID type for a temperature (also used as an index in the temperature db)
typedef uint16_t temperature_id_t;

// Temperature type definition
// In practice, represent the raw input coming from an LM35 sensor, but the
// data type could be changed in any moment to store additional informations
typedef uint16_t temperature_t;

// Temperature database type definition
typedef struct _temperature_db_s {
  temperature_id_t used;
  uint8_t id;
  uint8_t locked; // If 'locked', no other temperatures can be added
  temperature_t items[];
} temperature_db_t;

// Treat the NVM allocated space as a sequence of immutable DB objects
typedef temperature_db_t temperature_db_seq_t;


// Setup for using the temperature database
// Returns 0 on success, 1 if there is no space left for a new database
uint8_t temperature_init(void);

// Lock the DB currently in use and create a new one
// Returns 0 on success, 1 on insufficient space
uint8_t temperature_db_new(void);

// Register a new temperature in the database currently in use
// Returns 0 on success, 1 otherwise (e.g. if there is no more space)
uint8_t temperature_register(uint16_t raw_val);

// Get a temperature by id
// Returns 0 if the temperature exists, 1 otherwise
uint8_t temperature_get(uint8_t db_id, temperature_id_t temp_id,
    temperature_t *dest);

// Get 'ntemps' temperatures in bulk
// Returns the number of temperatures gotten, starting from 'start'
temperature_t temperature_get_bulk(uint8_t db_id, temperature_id_t start_id,
    temperature_id_t ntemps, temperature_t *dest);

// Returns the number of temperatures present in all the databases
temperature_id_t temperature_count_all(void);

// Returns the number of temperatures actually present in a database
temperature_id_t temperature_count(uint8_t db_id);

// Reset the database list, deleting all the temperatures
void temperature_db_reset(void);

#endif    // __TEMPERATURE_DB_H
