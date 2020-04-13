// AVR Temperature Monitor -- Paolo Lucchesi
// Non-Volatile Memory interface - Memory image
#include "temperature.h"
#include "lmsensor.h" // LM pin macros
#include "buttons.h" // Buttons macros
#include "config.h"
#include "nvm.h"


// The memory image itself
nvm_image_t NVMMEM _nvm_image = {
  .config = {
    .temperature_timer_resolution = 1000,
    .temperature_timer_interval = 2,
    .lmsensor_pin = A0,
    .btn_debounce_time = 20,
    .poweroff_pin = D21,
    .start_pin = D53,
    .stop_pin = D52
  },
  .db_seq = (temperature_db_seq_t) { 0 }
};


// Pointer to the memory image, relative to the .eeprom section
nvm_image_t *_nvm_image_ptr = &_nvm_image;
