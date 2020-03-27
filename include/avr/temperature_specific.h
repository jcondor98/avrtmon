// avrtmon
// AVR-side specific temperature interface - Head file
// Paolo Lucchesi - Wed 21 Aug 2019 04:14:15 PM CEST
#ifndef __TEMPERATURE_AVR_H
#define __TEMPERATURE_AVR_H

#ifndef __TEMPERATURE_INTERFACE_H
#error "Do not use implementation-specific temperature modules directly. Instead, '#include \"temperature.h\"'"
#endif

// Temperature database type definition
// A temperature is registered every rto_resolution * reg_interval seconds
typedef struct _temperature_db_s {
  temperature_id_t used;
  uint16_t reg_resolution; // Registration timer resolution
  uint16_t reg_interval;   // Registration timer interval
  uint8_t id;
  uint8_t locked; // If 'locked', no other temperatures can be added
  temperature_t items[];
} temperature_db_t;

// Treat the NVM allocated space as a sequence of immutable DB objects
typedef temperature_db_t temperature_db_seq_t;


// Setup for using the temperature database
void temperature_init(void);

// Lock the DB currently in use and create a new one
// Returns 0 on success, 1 on insufficient space
uint8_t temperature_db_new(uint16_t reg_resolution, uint16_t reg_interval);

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

// Craft a 'temperature_db_info_t' struct from an existent database
// Returns 0 on success, 1 if 'dest' is not valid or the DB does not exist
uint8_t temperature_db_info(uint8_t db_id, temperature_db_info_t dest);

#endif  // __TEMPERATURE_AVR_H
