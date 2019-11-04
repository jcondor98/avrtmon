// avrtmon
// Temperature database - Source file
// Paolo Lucchesi - Wed 30 Oct 2019 12:24:53 AM CET
#include <stdio.h>
#include <stdlib.h>
#include "temperature.h"


// Create a new, empty temperature database
// Returns a pointer to the new database on success, NULL otherwise
temperature_db_t *temperature_db_new(unsigned id, unsigned size) {
  if (!size) return NULL;

  temperature_db_t *db = malloc(sizeof(temperature_db_t));
  if (!db) return NULL;

  *db = (temperature_db_t) {
    .id = id,
    .size = size,
    .items = malloc(size * sizeof(float))
  };
  if (db->items) return db;

  // Allocation of 'items' was unsuccessful, handle the error
  free(db);
  return NULL;
}


// Delete (i.e. destroy) a temperature database
void temperature_db_delete(temperature_db_t *db) {
  if (!db) return;
  free(db->items);  // db->items is assumed to be valid
  free(db);
}


// Register a temperature, given its (wanted) id and its value
// Returns 0 on success, 1 otherwise
int temperature_register(temperature_db_t *db, unsigned id, float value) {
  if (!db || id >= db->size) return 1;
  db->items[id] = value;
  return 0;
}


// Get a temperature given its id, storing it into 'dest'
// Returns 0 on success, 1 otherwise
int temperature_get(temperature_db_t *db, unsigned id, float *dest) {
  if (!db || id >= db->size) return 1;
  *dest = db->items[id];
  return 0;
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

  // Write database metadata
  fprintf(out, "%u\n", db->id);
  fprintf(out, "%u\n", db->size);
  
  // Write the temperatures
  for (unsigned i=0; i < db->size; ++i)
    fprintf(out, "%f\n", db->items[i]);

  // Close the file
  return fclose(out) == 0 ? 0 : 1;
}
