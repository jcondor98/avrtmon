// avrtmon
// Non-Volatile Memory interface - Memory image
// Paolo Lucchesi - Fri 13 Sep 2019 01:41:38 AM CEST
#include "temperature.h"
#include "buttons.h" // Buttons macros
#include "config.h"
#include "nvm.h"


// The memory image itself
nvm_image_t NVMMEM _nvm_image = {
  .config = {
    .temperature_timer_resolution = 1,
    .temperature_timer_interval = 1,
    .start_pin = D53,
    .stop_pin = D52
  },
  .db_seq = (temperature_db_seq_t) { 0 }
};


// Pointer to the memory image, relative to the .eeprom section
nvm_image_t *_nvm_image_ptr = &_nvm_image;
