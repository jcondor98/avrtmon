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
    //FIELD-NVM-SUBST-HERE
  },
  // TODO: Make this dynamic (the setting below is a mere, temporary workaround)
  .db_seq = (temperature_db_seq_t) {
    .reg_resolution = 1000,
    .reg_interval = 2
  }
};


// Pointer to the memory image, relative to the .eeprom section
nvm_image_t *_nvm_image_ptr = &_nvm_image;
