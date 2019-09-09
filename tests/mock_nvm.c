// avrtmon
// Mock NVM Implementation - Source file
#include <string.h>
#include <stddef.h> // offsetof macro
#include <assert.h>

#include "nvm_mock.h"
#include "temperature.h"

char nvm[NVM_SIZE];


// Perform a generic initialization of the NVM with a buffer given by the user
// TODO: Protect the copy against overflow
void mock_nvm_init(const void *src, size_t size) {
  memcpy(nvm, src, size);
}

// TODO: Remove this and use the function above
void mock_nvm_init_for_temperature_db(void) {
  temperature_db_t db = { .capacity = TEMP_DB_CAPACITY };
  memcpy(nvm, &db, offsetof(temperature_db_t, items) - 1);
}



void nvm_read(void *dest, const void *src, size_t size) {
  if (size > NVM_SIZE) return;
  memcpy(dest, src, size);
}

void nvm_write(void *dest, const void *src, size_t size) {
  if (size > NVM_SIZE) return;
  memcpy(dest, src, size);
}
