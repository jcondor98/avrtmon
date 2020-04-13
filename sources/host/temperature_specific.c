// AVR Temperature Monitor -- Paolo Lucchesi
// Temperature database - Source file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "temperature.h"

#define MIN(x,y) ((x) > (y) ? (y) : (x))


// Create a new, empty temperature database
// Returns a pointer to the new database on success, NULL otherwise
temperature_db_t *temperature_db_new(unsigned id, unsigned size,
    unsigned reg_resolution, unsigned reg_interval, char *desc) {
  if (!size || !reg_resolution || !reg_interval) return NULL;

  temperature_db_t *db = malloc(sizeof(temperature_db_t));
  if (!db) return NULL;

  *db = (temperature_db_t) {
    .id = id,
    .size = size,
    .used = 0,
    .reg_resolution = reg_resolution,
    .reg_interval = reg_interval,
    .items = malloc(size * sizeof(float))
  };

  db->desc = desc ? strdup(desc) : NULL; // Ignore 'strdup()' eventual failure
  if (db->items) return db;

  // Allocation of 'items' was unsuccessful, handle the error
  if (db->desc) free(db->desc);
  free(db);
  return NULL;
}


// Delete (i.e. destroy) a temperature database
void temperature_db_delete(temperature_db_t *db) {
  if (!db) return;
  if (db->desc) free(db->desc);
  free(db->items);  // db->items is assumed to be valid
  free(db);
}

// Get the size of a temperature database
unsigned temperature_db_size(const temperature_db_t *db) {
  return db ? db->size : 0;
}

// Get the description of the temperature database, by copy
// At most dest_size-1 bytes will be copied
// Returns a pointer to 'dest' on success, NULL otherwise
// NOTE: The copied string won't be NULL-terminated if the desc is longer than
// the given buffer (see 'man strncpy')
char *temperature_db_get_desc(const temperature_db_t *db,
    char *dest, size_t dest_size) {
  return (db && db->desc && dest && dest_size) ?
    strncpy(dest, db->desc, dest_size) : NULL;
}

// Set the description of a database -- 'dest' will be duplicated
void temperature_db_set_desc(temperature_db_t *db, const char *desc) {
  if (!db) return;
  if (db->desc) free(db->desc);
  db->desc = desc ? strdup(desc) : NULL;
}

// Register a temperature, given its (wanted) id and its value
// Returns 0 on success, 1 otherwise
int temperature_register(temperature_db_t *db, float value) {
  if (!db || db->used >= db->size) return 1;
  db->items[db->used++] = value;
  return 0;
}

// Get a temperature given its id, storing it into 'dest'
// Returns 0 on success, 1 otherwise
int temperature_get(const temperature_db_t *db, unsigned id, float *dest) {
  if (!db || id >= db->size) return 1;
  *dest = db->items[id];
  return 0;
}

// Get temperatures in bulk
// Returns the number of temperatures gotten
unsigned temperature_get_bulk(const temperature_db_t *db, unsigned start_id,
    unsigned ntemps, float *dest) {
  if (!db || start_id >= db->used) return 0;
  unsigned to_get = db->used - start_id;
  to_get = MIN(to_get, ntemps);
  memcpy(dest, db->items + start_id, to_get * sizeof(float));
  return to_get;
}

// Get temperatures in bulk
// Returns the number of temperatures registered
unsigned temperature_register_bulk(temperature_db_t *db,
    unsigned ntemps, const float *src) {
  if (!db || db->used >= db->size) return 0;
  unsigned to_reg = db->size - db->used;
  to_reg = MIN(to_reg, ntemps);
  memcpy(db->items + db->used, src, to_reg * sizeof(float));
  db->used += to_reg;
  return to_reg;
}


// Export a temperature database as a text file containing newline separated
// strings, representing the temperatures.
// The first two lines are respectively the ID and the size
// Returns 0 on success, 1 otherwise
int temperature_db_export(const temperature_db_t *db, const char *fpath) {
  if (!db || !fpath) return 1;

  // Try to open 'fpath'
  FILE *out = fopen(fpath, "w");
  if (!out) {
    perror("Couldn't open output file in 'temperature_db_export'");
    return 1;
  }

  // Print database title
  if (!db->desc)
    fprintf(out, "Database %u\n", db->id);
  else fprintf(out, "%s\n", db->desc);

  // Compute time interval between temperature samples, in seconds
  printf("I-R: %hd %hd", db->reg_resolution, db->reg_interval);
  const double interval = ((double) db->reg_resolution * (double) db->reg_interval) / 1000;
  
  // Write the temperatures
  for (unsigned i=0; i < db->size; ++i)
    fprintf(out, "%.3g %.1f\n", interval * i, db->items[i]);

  // Close the file
  return fclose(out) == 0 ? 0 : 1;
}


// Convert a raw temperature coming from the avrtmon to a float
float temperature_raw2float(uint16_t raw) { return ((float) raw) / 10; }


// Print a database
void temperature_db_print(const temperature_db_t *db) {
  if (!db)
    printf("Tried to print a NULL temperature database\n");
  else
    printf("Database %u\n%s\nSize: %u\nUsed: %u\nInterval (ms): %u\n\n",
        db->id, db->desc ? db->desc : "[No description]", db->size, db->used,
        db->reg_resolution * db->reg_interval);
}
