// avrtmon
// Mock NVM Implementation - Source file
#include <string.h>
#include <stddef.h> // offsetof macro
#include "nvm_mock.h"
#include "temperature.h"

char nvm[NVM_SIZE];

void mock_nvm_init_for_temperature_db(void) {
  temperature_db_t db = { .capacity = TEMP_DB_CAPACITY };
  memcpy(nvm, &db, offsetof(temperature_db_t, items) - 1);
}

void nvm_read(void *dest, const void *src, size_t size) {
  if ((size_t) dest + size > NVM_SIZE) return;
  memcpy(nvm + (size_t) dest, src, size);
}

void nvm_write(void *dest, const void *src, size_t size) {
  if ((size_t) dest + size > NVM_SIZE) return;
  memcpy(nvm + (size_t) dest, src, size);
}
