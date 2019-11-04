// avrtmon
// Temperature database - Head file
// Paolo Lucchesi - Wed 30 Oct 2019 12:12:31 AM CET
#ifndef __TEMPERATURE_DB_H
#define __TEMPERATURE_DB_H
#include <stdint.h>

// Type definition for a single temperature database
typedef struct _temperature_db_s {
  unsigned id;    // ID of the database (multiple databases will be stored)
  unsigned size;
  float *items;
} temperature_db_t;


// Create a new, empty temperature database
// Returns a pointer to the new database on success, NULL otherwise
temperature_db_t *temperature_db_new(unsigned id, unsigned size);

// Delete (i.e. destroy) a temperature database
void temperature_db_delete(temperature_db_t *db);

// Register a temperature, given its (wanted) id and its value
// Returns 0 on success, 1 otherwise
int temperature_register(temperature_db_t *db, unsigned id, float value);

// Get a temperature given its id, storing it into 'dest'
// Returns 0 on success, 1 otherwise
int temperature_get(temperature_db_t *db, unsigned id, float *dest);

// Export a temperature database as a text file containing newline separated
// strings, representing the temperatures.
// The first two lines are respectively the ID and the size
// Returns 0 on success, 1 otherwise
int temperature_export(const char *fpath);


// Convert a raw temperature (i.e. uint16_t) coming from the avrtmon to a float
// TODO: Do a proper conversion
float temperature_raw2float(uint16_t raw);

#endif  // __TEMPERATURE_DB_H
