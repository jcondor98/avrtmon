// avrtmon
// Non-Volatile Memory interface - Memory image
// Paolo Lucchesi - Fri 13 Sep 2019 01:41:38 AM CEST
#ifndef TEST
#include "nvm.h"
#else
#include "nvm_mock.h"
#endif

#include "temperature.h"
#include "config.h"


// The memory image itself
nvm_image_t NVMMEM _nvm_image = {
  .config = {
    .field1 = 1,
    .field2 = 2,
    .field3 = 3
  },
  .db = {
    .capacity = TEMP_DB_CAPACITY,
    .used = 0,
    .items = _nvm_image.db_items
  }
};


// Pointer to the memory image, relative to the .eeprom section
nvm_image_t *nvm_image = &_nvm_image;
