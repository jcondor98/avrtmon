// avrtmon
// Temperature database - Source file
// Paolo Lucchesi - Wed 21 Aug 2019 04:14:06 PM CEST
#include <stddef.h>  // offsetof macro
#include <string.h>  // memcpy

#include "temperature.h"
#ifdef TEST
#include "nvm_mock.h"
#else
#include "nvm.h"
#endif


// Get the address of an arbitrary field in an NVM, passing the field name
#define NVM_ADDR_FIELD(field) \
  (((void*) &nvm_image->db) + offsetof(temperature_db_t, field))

// Get the address of a single temperature, given its ID, in the NVM
#define NVM_ADDR_ITEM(id) (((temperature_t*) nvm_image->db_items) + id)


// Fetch the entire database from the NVM
static void _temperature_fetch_entire_db_from_nvm(temperature_db_t *dest_db,
    temperature_t *dest_items);


// Database Cache in memory
static temperature_db_t cache_db;
#ifdef TEMP_DB_CACHE
static temperature_t cache_db_items[TEMP_DB_CAPACITY];
#endif


// Setup for using the temperature database
// Does nothing if DB caching is not enabled
void temperature_init(void) {
#ifdef TEMP_DB_CACHE
  _temperature_fetch_entire_db_from_nvm(&cache_db, cache_db_items);
#else
  _temperature_fetch_entire_db_from_nvm(&cache_db, NULL);
#endif
}


// Register a new temperature
// Returns 0 on success, 1 otherwise (e.g. if there is no more space)
uint8_t temperature_register(uint16_t raw_val) {
  // Check for available space in the database
  if (cache_db.used >= TEMP_DB_CAPACITY)
    return 1;

  // Prepare the temperature to be written to the NVM
  temperature_t *t;

#ifdef TEMP_DB_CACHE
  t = cache_db.items + cache_db.used;
#else
  temperature_t _t;
  t = &_t;
#endif

  memcpy(t, &raw_val, sizeof(temperature_t));

  // Effectively register the temperature
  nvm_busy_wait(); // NVM must be available before writing
  nvm_write(NVM_ADDR_ITEM(cache_db.used), t, sizeof(temperature_t));
  nvm_busy_wait();

  cache_db.used++;
  nvm_write(NVM_ADDR_FIELD(used), &cache_db.used, sizeof(id_t));

  return 0; // Do not wait for the write operation to be finished
}


// Get a temperature by id
// Returns 0 if the temperature exists, 1 otherwise (or if 'dest' is not valid)
// If the temperature exists, it is copied in *dest
uint8_t temperature_get(id_t id, temperature_t *dest) {
  if (!dest) return 1;

  // Check for the existence of the temperature
  if (id >= cache_db.used) return 1;

  // Fetch the temperature in the database
  temperature_t *src;

#ifdef TEMP_DB_CACHE
  src = cache_db.items + id;
#else
  temperature_t _src;
  src = &_src;
  nvm_busy_wait();
  nvm_read(NVM_ADDR_ITEM(id), src, sizeof(temperature_t));
  nvm_busy_wait();
#endif

  memcpy(dest, src, sizeof(temperature_t));
  return 0;
}


// Returns the number of temperatures actually present in the database
id_t temperature_count(void) { return cache_db.used; }

// Returns the capacity of the database (in registerable temperatures)
id_t temperature_capacity(void) { return cache_db.capacity; }


// Reset the database, deleting all the temperatures
void temperature_db_reset(void) {
  cache_db.used = 0;
  id_t zero = 0;
  nvm_busy_wait();
  nvm_write(NVM_ADDR_FIELD(used), &zero, sizeof(id_t));
}


// Copy the entire database in a data structure provided by the user
// Returns 0 on success, 1 otherwise
uint8_t temperature_fetch_entire_db(temperature_db_t *dest_db,
    temperature_t *dest_items) {

  if (!dest_db) return 1;

  // Copy the db metadata
  memcpy(dest_db, &cache_db, sizeof(temperature_db_t));

  // Copy the db items buffer if it is cached, else fetch it
  if (dest_items) {
#ifdef TEMP_DB_CACHE
    memcpy(dest_items, cache_db_items, cache_db.used);
#else
    nvm_busy_wait();
    nvm_read(dest_items, &nvm_image->db_items,
        cache_db.used * sizeof(temperature_t));
    nvm_busy_wait();
#endif
  }

  dest_db->items = dest_items;
  return 0;
}


// Fetch the entire database from the NVM
static void _temperature_fetch_entire_db_from_nvm(temperature_db_t *dest_db,
    temperature_t *dest_items) {

  // Get the DB metadata
  nvm_busy_wait();
  nvm_read(dest_db, &nvm_image->db, sizeof(temperature_db_t));

  // If 'dest_db_items' is a valid pointer, fetch the entire items buffer too
  if (dest_items) {
    nvm_busy_wait();
    nvm_read(dest_items, &nvm_image->db_items,
        cache_db.used * sizeof(temperature_t));
    dest_db->items = dest_items;
  }
  else dest_db->items = NULL;

  nvm_busy_wait();
}
