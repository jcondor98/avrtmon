// avrtmon
// Temperature database - Source file
// Paolo Lucchesi - Wed 21 Aug 2019 04:14:06 PM CEST
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
static void *_item_nvm_addr(void *db_nvm_addr, temperature_id_t t_id);
static void *_db_nvm_addr_next(const cache_db_t *current);
static void *_db_nvm_addr(uint8_t db_id);
static uint8_t _db_fetch_next(cache_db_t *dest, const cache_db_t *current);
static uint8_t _db_fetch_by_id(cache_db_t *dest, uint8_t db_id);
static uint8_t _db_new(void);
static void _db_fetch(cache_db_t *dest, void *nvm_addr);
static void _db_sync(const cache_db_t *db);
#define _db_load(nvm_addr) _db_fetch(&local_db, nvm_addr)
#define _db_load_next() _db_fetch_next(&local_db, &local_db)


// Setup for using the temperature database
uint8_t temperature_init(void) {
  // Load first database in memory
  _db_load(&nvm_image->db_seq);

  // Determine the last used database
  while (local_db.meta.locked)
    if (_db_load_next() != 0) {
      return 1; // TODO: Out of NVM memory
    }

  // Now, the last used DB is loaded; lock it and create a new one
  if (_db_new() != 0) {
    return 1; // TODO: Out of NVM memory
  }

  // TODO: Start registering?
  return 0;
}


// Lock the DB currently in use and create a new one
// Returns 0 on success, 1 on insufficient space
uint8_t temperature_db_new(void) {
  // TODO: Stop registering
  return _db_new();
}


// Register a new temperature
// Returns 0 on success, 1 otherwise (e.g. if there is no more space)
uint8_t temperature_register(uint16_t raw_val) {
  if (_item_nvm_addr(local_db.nvm_addr, local_db.meta.used)
      + sizeof(temperature_t) - 1 > NVM_LIMIT)
    return 1;

  nvm_busy_wait();
  nvm_write(_item_nvm_addr(local_db.nvm_addr, local_db.meta.used),
      &raw_val, sizeof(temperature_t));
  local_db.meta.used++;
  nvm_busy_wait(); // Useless, but makes the code resilient. Better safe than sorry

  nvm_write(NVM_ADDR_FIELD(local_db.nvm_addr, used),
      &local_db.meta.used, sizeof(local_db.meta.used));
  nvm_busy_wait();

  return 0;
}


// Get a temperature by id
// Returns 0 if the temperature exists, 1 otherwise
uint8_t temperature_get(uint8_t db_id, temperature_id_t temp_id,
    temperature_t *dest) {
  if (!dest || _db_fetch_by_id(&local_db_aux, db_id) != 0 ||
      temp_id >= local_db_aux.meta.used)
    return 1;

  nvm_busy_wait();
  nvm_read(dest, _item_nvm_addr(local_db_aux.nvm_addr, temp_id),
      sizeof(temperature_t));
  nvm_busy_wait();

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

  temperature_t to_read = local_db_aux.meta.used - start_id; // TODO (but should be fine)
  to_read = to_read >= ntemps ? ntemps : to_read;
  nvm_busy_wait();
  nvm_read(dest, _item_nvm_addr(local_db_aux.nvm_addr, start_id),
      to_read * sizeof(temperature_t));
  nvm_busy_wait();

  return to_read;
}


// Returns the number of temperatures actually present in a database
temperature_id_t temperature_count(uint8_t db_id) {
  if (db_id == local_db.meta.id) return local_db.meta.used;

  void *nvm_addr = _db_nvm_addr(db_id);
  if (!nvm_addr) return 0;

  nvm_busy_wait();
  temperature_id_t used;
  nvm_read(&used, NVM_ADDR_FIELD(nvm_addr, used), sizeof(temperature_t));
  nvm_busy_wait();

  return used;
}


// Returns the number of temperatures present in all the databases
temperature_id_t temperature_count_all(void) {
  temperature_id_t count = 0;

  _db_fetch(&local_db_aux, &nvm_image->db_seq);
  while (local_db_aux.meta.locked) {
    count += local_db_aux.meta.used;
    _db_fetch_next(&local_db_aux, &local_db_aux);
  }

  count += local_db_aux.meta.used;
  return count;
}


// Reset the database sequence, deleting all the temperatures
void temperature_db_reset(void) {
  // TODO: Stop registering
  local_db.nvm_addr = &nvm_image->db_seq;
  local_db.meta = (temperature_db_t) { 0 };
  _db_sync(&local_db);
}


// Craft a 'temperature_db_info_t' struct from an existent database
// Returns 0 on success, 1 if some parameter is not valid
uint8_t temperature_db_info(uint8_t db_id, temperature_db_info_t dest) {
  if (!dest || _db_fetch_by_id(&local_db_aux, db_id) != 0)
    return 1;
  temperature_db_info_pack(dest, local_db_aux.meta.id, local_db_aux.meta.used);
  return 0;
}



// [AUX] Lock the DB loaded in 'local_db' in the NVM and create a new one
// Returns 0 on success, 1 on insufficient space
// WARNING: Make sure that temperature registering is not ongoing
// NOTE: If the last database is empty, consider it as the new one
static uint8_t _db_new(void) {
  if (local_db.meta.used == 0) return 0; // No need to create another DB

  // Lock currently loaded DB
  if (!local_db.meta.locked) {
    local_db.meta.locked = 1;
    _db_sync(&local_db);
  }

  // Compute next DB address and check against insufficient space
  void *nvm_next = _db_nvm_addr_next(&local_db);
  if (!nvm_next) return 1; // No more space

  // Create the new database
  local_db.nvm_addr = nvm_next;
  local_db.meta.locked = 0;
  local_db.meta.used = 0;
  local_db.meta.id++;
  _db_sync(&local_db);

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
        current->meta.used)); // TODO

  if (next + sizeof(temperature_db_t) + sizeof(temperature_t) >
      NVM_LIMIT - TEMP_DB_MIN_SIZE)
    return NULL;
  else return next;
}


// [AUX] Fetch an entire database given its base NVM address
static void _db_fetch(cache_db_t *dest, void *nvm_addr) {
  nvm_busy_wait();
  nvm_read(&dest->meta, nvm_addr, sizeof(temperature_db_t));
  dest->nvm_addr = nvm_addr;
  nvm_busy_wait();
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
static void *_item_nvm_addr(void *db_nvm_addr, temperature_id_t t_id) {
  return ((temperature_db_t*) db_nvm_addr)->items + t_id;
}


// [AUX] Synchronize the modifications between the cache and the actual NVM
static void _db_sync(const cache_db_t *db) {
  nvm_busy_wait();
  nvm_update(db->nvm_addr, &db->meta, sizeof(temperature_db_t));
  nvm_busy_wait();
}
