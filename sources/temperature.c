// avrtmon
// Temperature database - Source file
// Paolo Lucchesi - Wed 21 Aug 2019 04:14:06 PM CEST
#include <stddef.h>  // offsetof macro
#include <string.h>  // memcpy
#include "temperature.h"


// Get the address of an arbitrary field in an NVM, passing the field name
#define NVM_ADDR_FIELD(field) \
  ((void*)(TEMP_DB_OFFSET + offsetof(temperature_db_t, field)))

// Get the address of a single temperature, given its ID, in the NVM
#define NVM_ADDR_ITEM(id) \
  ((void*)(TEMP_DB_OFFSET + offsetof(temperature_db_t, items) + \
   ((id) * sizeof(temperature_t))))


// Get the 'used' field of the temperature
static inline id_t _db_get_used(void);

// Get the 'capacity' field of the database
static inline id_t _db_get_capacity(void);

// Fetch the entire database from the NVM
static void _temperature_fetch_entire_db_from_nvm(temperature_db_t *dest_db);


// Database Cache in memory
#ifdef TEMP_DB_CACHE
static temperature_db_t cache_db;
#endif


// Setup for using the temperature database
// Does nothing if DB caching is not enabled
void temperature_init(void) {
#ifdef TEMP_DB_CACHE
  _temperature_fetch_entire_db_from_nvm(&cache_db);
#endif
}


// Register a new temperature
// Returns 0 on success, 1 otherwise (e.g. if there is no more space)
uint8_t temperature_register(uint16_t raw_val) {
  // Check for available space in the database
  id_t used = _db_get_used();
  if (used >= TEMP_DB_CAPACITY)
    return 1;

  // Prepare the temperature to be written to the NVM
  temperature_t *t;

#ifdef TEMP_DB_CACHE
  t = cache_db.items + used;
  cache_db.used++;
#else
  temperature_t _t;
  t = &_t;
#endif

  // TODO: Make this resilient to a change of the temperature_t type
  memcpy(t, &raw_val, sizeof(temperature_t));

  // Effectively register the temperature
  nvm_busy_wait(); // NVM must be available before writing
  nvm_write(NVM_ADDR_ITEM(used), t, sizeof(temperature_t));
  nvm_busy_wait();

  ++used;
  nvm_write(NVM_ADDR_FIELD(used), &used, sizeof(id_t));

  return 0; // Do not wait for the write operation to be finished
}


// Get a temperature by id
// Returns 0 if the temperature exists, 1 otherwise (or if 'dest' is not valid)
// If the temperature exists, it is copied in *dest
uint8_t temperature_get(id_t id, temperature_t *dest) {
  if (!dest) return 1;

  // Check for the existence of the temperature
  id_t used = _db_get_used();
  if (id >= used) return 1;

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
id_t temperature_count(void) {
  return _db_get_used();
}

// Returns the capacity of the database (in registerable temperatures)
id_t temperature_capacity(void) {
  return _db_get_capacity();
}


// Reset the database, deleting all the temperatures
void temperature_db_reset(void) {
#ifdef TEMP_DB_CACHE
  cache_db.used = 0;
#endif

  id_t zero = 0;
  nvm_busy_wait();
  nvm_write(NVM_ADDR_FIELD(used), &zero, sizeof(id_t));
}


// Copy the entire database in a data structure provided by the user
// Returns 0 on success, 1 otherwise
uint8_t temperature_fetch_entire_db(temperature_db_t *dest_db) {
  if (!dest_db) return 1;

#ifdef TEMP_DB_CACHE
  memcpy(dest_db, &cache_db, sizeof(temperature_db_t));
#else
  _temperature_fetch_entire_db_from_nvm(dest_db);
#endif

  return 0;
}


// Fetch the entire database from the NVM
static void _temperature_fetch_entire_db_from_nvm(temperature_db_t *dest_db) {
  // Get the database metadata, which precedes the items array
  nvm_busy_wait();
  nvm_read(dest_db, NVM_ADDR_ITEM(0) - 1,
      TEMP_DB_OFFSET + offsetof(temperature_db_t, items) - 1);

  // Get all the temperatures
  nvm_busy_wait();
  nvm_read(((void*) dest_db) + offsetof(temperature_db_t, items),
      NVM_ADDR_ITEM(0), dest_db->used * sizeof(temperature_t));
}


// Get the 'used' field of the database
static inline id_t _db_get_used(void) {
#ifdef TEMP_DB_CACHE
  return cache_db.used;
#else
  id_t used;
  nvm_busy_wait();
  nvm_read(NVM_ADDR_FIELD(used), &used, sizeof(id_t));
  nvm_busy_wait();
  return used;
#endif
}

// Get the 'capacity' field of the database
static inline id_t _db_get_capacity(void) {
#ifdef TEMP_DB_CACHE
  return cache_db.capacity;
#else
  id_t capacity;
  nvm_busy_wait();
  nvm_read(NVM_ADDR_FIELD(capacity), &capacity, sizeof(id_t));
  nvm_busy_wait();
  return capacity;
#endif
}
