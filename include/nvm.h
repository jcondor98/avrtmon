// avrtmon
// Non-Volatile Memory interface (MCU: Atmel ATMega2560)
// Paolo Lucchesi - Wed 21 Aug 2019 04:43:08 PM CEST
// NOTE: the order of the arguments has been changed to gain consistency across
// the interface and to fulfill UNIX standard functions (e.g. memcpy)
#ifndef __NVM_INTERFACE_H
#define __NVM_INTERFACE_H
#include <avr/eeprom.h>

#define NVM_SIZE 4096
#define NVM_LIMIT ((void*)NVM_SIZE)

// Put a variable in the NVM image, if the operation is supported
#define NVMMEM EEMEM

// Read a block of data to the NVM
#define nvm_read(dst,src,size) \
  eeprom_read_block(dst,src,size)

// Write a block of data to the NVM
#define nvm_write(dst,src,size) \
  eeprom_write_block(src,dst,size)

// Write a block of data to the NVM only if it differs from the existent one
// Note that the order of the arguments has been changed to gain consistency
// across the interface and to fulfill UNIX standard functions (e.g. memcpy)
#define nvm_update(dst,src,size) \
  eeprom_update_block(src,dst,size)

// Is there an ongoing operation?  0 -> No, !0 -> Yes
#define nvm_ongoing() (!eeprom_ready())

// Do nothing while an NVM operation is ongoing
#define nvm_busy_wait() eeprom_busy_wait()

#endif    // __NVM_INTERFACE_H
