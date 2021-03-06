// AVR Temperature Monitor -- Paolo Lucchesi
// Temperature database - Source file
#include <stddef.h>  // offsetof macro
#include <string.h>  // memcpy

#include "temperature.h"
#include "nvm.h"


// Cache memory to avoid an excessive number of NVM read operations
// WARNING: Do NOT try to access items from the 'meta' structure!
// 'local_db_aux' is used to make 'cache_db' independent from any use of any
// auxiliary function (e.g.: download all the temperatures while registering)
typedef struct _cache_db_s {
  temperature_db_t meta;  // Metadata of the currently loaded DB
  void *nvm_addr;         // NVM base address of the currently loaded DB
} cache_db_t;
cache_db_t local_db, local_db_aux;

// Room necessary for a DB with just one temperature
#define TEMP_DB_MIN_SIZE (sizeof(temperature_db_t) + sizeof(temperature_t))

// Get the address of an arbitrary field in an NVM, passing the field name
#define NVM_ADDR_FIELD(db_nvm_addr, field) \
  (((void*) db_nvm_addr) + offsetof(temperature_db_t, field))


// Auxiliary functions -- See at the bottom of this source file
static inline void *_item_nvm_addr(void *db_nvm_addr, temperature_id_t t_id);
static inline void *_db_nvm_addr(uint8_t db_id);
static void *_db_nvm_addr_next(const cache_db_t *current);
static uint8_t _db_fetch_next(cache_db_t *dest, const cache_db_t *current);
static uint8_t _db_fetch_by_id(cache_db_t *dest, uint8_t db_id);
static inline void _db_fetch(cache_db_t *dest, void *nvm_addr);
static inline void _db_sync(const cache_db_t *db);
#define _db_load(nvm_addr) _db_fetch(&local_db, nvm_addr)
#define _db_load_next() _db_fetch_next(&local_db, &local_db)


// Setup for using the temperature database
// This just loads the last used DB
void temperature_init(void) {
  // Load the last database in memory, locked or not
  _db_load(&nvm_image->db_seq); // <- First DB
  while (local_db.meta.locked && _db_load_next() != 0)
    ;
}


// Lock the DB currently in use and create a new one
// Returns 0 on success, 1 on insufficient space
uint8_t temperature_db_new(uint16_t reg_resolution, uint16_t reg_interval) {
  if (local_db.meta.used == 0) { // No need to create new DB, use current one
    local_db.meta.locked = 0;
    local_db.meta.reg_resolution = reg_resolution;
    local_db.meta.reg_interval = reg_interval;
    _db_sync(&local_db);
    return 0;
  }

  // Lock currently loaded DB
  if (!local_db.meta.locked) {
    local_db.meta.locked = 1;
    _db_sync(&local_db);
  }

  // Compute next DB address and check against insufficient space
  void *nvm_next = _db_nvm_addr_next(&local_db);
  if (!nvm_next) return 1; // No more space

  // Create the new database
  local_db.meta.used = 0;
  local_db.meta.reg_resolution = reg_resolution;
  local_db.meta.reg_interval = reg_interval;
  local_db.meta.id++;
  local_db.meta.locked = 0;
  local_db.nvm_addr = nvm_next;
  _db_sync(&local_db);

  return 0;
}


// Register a new temperature
// Returns 0 on success, 1 otherwise (e.g. if there is no more space)
uint8_t temperature_register(uint16_t raw_val) {
  if (local_db.meta.locked || _item_nvm_addr(local_db.nvm_addr, local_db.meta.used)
      + sizeof(temperature_t) - 1 > NVM_LIMIT)
    return 1;

  nvm_update(_item_nvm_addr(local_db.nvm_addr, local_db.meta.used),
      &raw_val, sizeof(temperature_t));
  local_db.meta.used++;

  nvm_update(NVM_ADDR_FIELD(local_db.nvm_addr, used),
      &local_db.meta.used, sizeof(local_db.meta.used));

  return 0;
}


// Get a temperature by id
// Returns 0 if the temperature exists, 1 otherwise
uint8_t temperature_get(uint8_t db_id, temperature_id_t temp_id,
    temperature_t *dest) {
  if (!dest || _db_fetch_by_id(&local_db_aux, db_id) != 0 ||
      temp_id >= local_db_aux.meta.used)
    return 1;

  nvm_read(dest, _item_nvm_addr(local_db_aux.nvm_addr, temp_id),
      sizeof(temperature_t));

  return 0;
}


// Get 'ntemps' temperatures in bulk
// Returns the number of temperatures gotten, starting from 'start'
// Could return 0 or less than 'ntemps' if there are no or less temperatures
// present in the DB
temperature_t temperature_get_bulk(uint8_t db_id, temperature_id_t start_id,
    temperature_id_t ntemps, temperature_t *dest) {
  if (!dest || _db_fetch_by_id(&local_db_aux, db_id) != 0 ||
      start_id >= local_db_aux.meta.used)
    return 0;

  temperature_id_t to_read = local_db_aux.meta.used - start_id;
  to_read = to_read >= ntemps ? ntemps : to_read;
  nvm_read(dest, _item_nvm_addr(local_db_aux.nvm_addr, start_id),
      to_read * sizeof(temperature_t));

  return to_read;
}


// Returns the number of temperatures actually present in a database
temperature_id_t temperature_count(uint8_t db_id) {
  if (db_id == local_db.meta.id) return local_db.meta.used;

  void *nvm_addr = _db_nvm_addr(db_id);
  if (!nvm_addr) return 0;

  temperature_id_t used;
  nvm_read(&used, NVM_ADDR_FIELD(nvm_addr, used), sizeof(temperature_t));

  return used;
}


// Returns the number of temperatures present in all the databases
temperature_id_t temperature_count_all(void) {
  temperature_id_t count = 0;

  _db_fetch(&local_db_aux, &nvm_image->db_seq);
  while (local_db_aux.meta.locked) {
    count += local_db_aux.meta.used;
    if (_db_fetch_next(&local_db_aux, &local_db_aux) != 0)
      return count;
  }

  return count + local_db_aux.meta.used; // Last DB was not locked
}


// Reset the database sequence, deleting all temperatures
void temperature_db_reset(void) {
  uint16_t last_res = local_db.meta.reg_resolution;
  uint16_t last_int = local_db.meta.reg_interval;
  local_db.nvm_addr = &nvm_image->db_seq;
  local_db.meta = (temperature_db_t) {
    .reg_resolution = last_res, .reg_interval = last_int, .used = 0, .locked = 0
  };
  _db_sync(&local_db);
}


// Craft a 'temperature_db_info_t' struct from an existent database
// Returns 0 on success, 1 if some parameter is not valid
uint8_t temperature_db_info(uint8_t db_id, temperature_db_info_t dest) {
  if (!dest || _db_fetch_by_id(&local_db_aux, db_id) != 0)
    return 1;
  temperature_db_info_pack(dest, local_db_aux.meta.id, local_db_aux.meta.used,
      local_db_aux.meta.reg_resolution, local_db_aux.meta.reg_interval);
  return 0;
}



// [AUX] Get the NVM memory address of an arbitrary temperature database
// Returns NULL on failure
static void *_db_nvm_addr(uint8_t db_id) {
  if (local_db.meta.id == db_id) return local_db.nvm_addr;
  if (local_db.meta.id < db_id)
    memcpy(&local_db_aux, &local_db, sizeof(local_db));
  else
    _db_fetch(&local_db_aux, &nvm_image->db_seq);

  // Attempt to retrieve the db_id-th database
  while (local_db_aux.meta.id != db_id) {
    void *nvm_next = _db_nvm_addr_next(&local_db_aux);
    if (!nvm_next) return NULL;
    _db_fetch(&local_db_aux, nvm_next);
  }

  return local_db_aux.nvm_addr;
}


// [AUX] Get the next temperature DB NVM address, or 0 if no other is present
// Returns NULL if the currently loaded database is the last one
static void *_db_nvm_addr_next(const cache_db_t *current) {
  if (!current->meta.locked) return NULL;
  void *next = ((void*) (((temperature_db_t*) current->nvm_addr)->items +
        current->meta.used));

  if (next + sizeof(temperature_db_t) + sizeof(temperature_t) >
      NVM_LIMIT - TEMP_DB_MIN_SIZE)
    return NULL;
  else return next;
}


// [AUX] Fetch an entire database given its base NVM address
static inline void _db_fetch(cache_db_t *dest, void *nvm_addr) {
  nvm_read(&dest->meta, nvm_addr, sizeof(temperature_db_t));
  dest->nvm_addr = nvm_addr;
}


// [AUX] Given a database in 'cache_db', load the next db in the cache
// Returns 0 on success, 1 if the currently loaded database is the last one
// 'dest' and 'current' can point to the same area
static uint8_t _db_fetch_next(cache_db_t *dest, const cache_db_t *current) {
  void *nvm_next = _db_nvm_addr_next(current);
  if (!nvm_next) return 1;
  _db_fetch(dest, nvm_next);
  return 0;
}


// [AUX] Fetch the entire database from the NVM
// Returns 0 on success, 1 on failure
static uint8_t _db_fetch_by_id(cache_db_t *dest, uint8_t db_id) {
  void *nvm_addr = _db_nvm_addr(db_id);
  if (!nvm_addr) return 1;
  _db_fetch(dest, nvm_addr);
  return 0;
}


// [AUX] Get the NVM address of a temperature item (giving its NVM DB address)
// Deliberately don't check for errors
static inline void *_item_nvm_addr(void *db_nvm_addr, temperature_id_t t_id) {
  return ((temperature_db_t*) db_nvm_addr)->items + t_id;
}


// [AUX] Synchronize the modifications between the cache and the actual NVM
static inline void _db_sync(const cache_db_t *db) {
  nvm_update(db->nvm_addr, &db->meta, sizeof(temperature_db_t));
}
