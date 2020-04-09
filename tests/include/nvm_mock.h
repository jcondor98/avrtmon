// AVR Temperature Monitor -- Paolo Lucchesi
// Mock NVM Implementation - Head file
#ifndef __NVM_INTERFACE_MOCK_H
#define __NVM_INTERFACE_MOCK_H

#ifndef __NVM_INTERFACE_H
#error "Cannot use mock NVM interface without base NVM module"
#endif

// EEMEM expands to nothing
#define NVMMEM

// Pointer to the (mock and emulated) NVM memory image
nvm_image_t *nvm_image;


// Initialize mock NVM module
// Every bit of uninitialized NVM data is set to 1, as in a real EEPROM
void nvm_mock_init(void);

// Read/Write functions
void _nvm_read(void *dest, const void *src, size_t size);
void _nvm_write(void *dest, const void *src, size_t size);

// Emulate the real NVM module, with out-of-the-box compatibility for your
// application modules
#define nvm_read _nvm_read
#define nvm_write _nvm_write
#define nvm_update _nvm_write
#define nvm_ongoing() 0;
#define nvm_busy_wait() do {} while (0)

#endif    // __NVM_INTERFACE_MOCK_H
