// avrtmon
// Host-side specific temperature interface - Head file
// Paolo Lucchesi - Wed 30 Oct 2019 12:12:31 AM CET
#ifndef __TEMPERATURE_HOST_H
#define __TEMPERATURE_HOST_H

#ifndef __TEMPERATURE_INTERFACE_H
#error "Do not use implementation-specific temperature modules directly. Instead, '#include \"temperature.h\"'"
#endif

// Type definition for a single temperature database
// A temperature is registered every rto_resolution * reg_interval milliseconds
// This data structure is intended to be constant; there are no function to
// modify or remove temperatures
typedef struct _temperature_db_s {
  unsigned id;    // ID of the database (multiple databases can be stored)
  unsigned size;
  unsigned used;
  unsigned reg_resolution; // Registration timer resolution
  unsigned reg_interval;   // Registration timer interval
  char *desc;     // Optional, brief description of the database
  float *items;
} temperature_db_t;


// Create a new, empty temperature database
// Returns a pointer to the new database on success, NULL otherwise
temperature_db_t *temperature_db_new(unsigned id, unsigned size,
    unsigned reg_resolution, unsigned reg_interval, char *desc);

// Delete (i.e. destroy) a temperature database
void temperature_db_delete(temperature_db_t *db);

// Get the size of a temperature database
unsigned temperature_db_size(const temperature_db_t *db);

// Get the description of a temperature database, by copy
// At most dest_size-1 bytes will be copied
char *temperature_db_get_desc(const temperature_db_t *db,
    char *dest, size_t dest_size);

// Set the description of a database -- 'dest' will be duplicated
void temperature_db_set_desc(temperature_db_t *db, const char *dest);

// Register a temperature
// Returns 0 on success, 1 otherwise
int temperature_register(temperature_db_t *db, float value);

// Get a temperature given its id, storing it into 'dest'
// Returns 0 on success, 1 otherwise
int temperature_get(const temperature_db_t *db, unsigned id, float *dest);

// Get temperatures in bulk
// Returns the number of temperatures gotten
unsigned temperature_get_bulk(const temperature_db_t *db, unsigned start_id,
    unsigned ntemps, float *dest);

// Get temperatures in bulk
// Returns the number of temperatures registered
unsigned temperature_register_bulk(temperature_db_t *db,
    unsigned ntemps, const float *src);

// Export a temperature database as a text file containing newline separated
// strings, representing the temperatures.
// The first two lines are respectively the ID and the size
// Returns 0 on success, 1 otherwise
int temperature_db_export(const temperature_db_t *db, const char *fpath);


// Convert a raw temperature (i.e. uint16_t) coming from the avrtmon to a float
// TODO: Do a proper conversion
float temperature_raw2float(uint16_t raw);

#endif  // __TEMPERATURE_HOST_H
