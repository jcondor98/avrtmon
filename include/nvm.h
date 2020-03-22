// avrtmon
// Non-Volatile Memory interface (MCU: Atmel ATMega2560)
// Paolo Lucchesi - Wed 21 Aug 2019 04:43:08 PM CEST
// NOTE: the order of the arguments has been changed to gain consistency across
// the interface and to fulfill UNIX standard functions (e.g. memcpy)
#ifndef __NVM_INTERFACE_H
#define __NVM_INTERFACE_H
#include <stddef.h>
#include "config.h"
#include "temperature.h"

// NVM size
#define NVM_SIZE 4096

// Data type definition for the memory image
// Defining an ad-hoc type ensures that the data is stored in a precise order
typedef struct _nvm_image_s {
  config_t config;
  temperature_db_t db;
  temperature_t db_items[];
} nvm_image_t;

// Temperature database capacity
#define TEMP_DB_CAPACITY ((NVM_SIZE - offsetof(nvm_image_t, db_items)) \
    / sizeof(temperature_t))

// Image size with and without the DB items buffer
#define NVM_IMAGE_META_SIZE offsetof(nvm_image_t, db)
#define NVM_IMAGE_FULL_SIZE (NVM_IMAGE_META_SIZE + \
    TEMP_DB_CAPACITY * sizeof(temperature_t))

// Pointer to the memory image
// It is safe to expose this as the image will be saved in the .eeprom section
nvm_image_t *nvm_image;


#ifdef TEST // Use mock interface when testing
#include "nvm_mock.h"

#else  // AVR real NVM interface
#include <avr/eeprom.h>

// Put a variable in the NVM image, if the operation is supported
#define NVMMEM EEMEM

// Read a block of data to the NVM
#define nvm_read(dst,src,size) eeprom_read_block(dst,src,size)

// Write a block of data to the NVM
#define nvm_write(dst,src,size) eeprom_write_block(src,dst,size)

// Write a block of data to the NVM only if it differs from the existent one
// Note that the order of the arguments has been changed to gain consistency
// across the interface and to fulfill UNIX standard functions (e.g. memcpy)
#define nvm_update(dst,src,size) eeprom_update_block(src,dst,size)

// Is there an ongoing operation?  0 -> No, !0 -> Yes
#define nvm_ongoing() (!eeprom_ready())

// Do nothing while an NVM operation is ongoing
#define nvm_busy_wait() eeprom_busy_wait()

#endif    // TEST/AVR

#endif    // __NVM_INTERFACE_H
