// AVR Temperature Monitor -- Paolo Lucchesi
// Mock NVM Implementation - Source file
#include <stdio.h>
#include <string.h>
#include <stddef.h> // offsetof macro

#ifndef TEST
#error "Cannot use mock NVM module when not testing. You should '#define TEST'"
#endif

#include "temperature.h"
#include "nvm.h"


unsigned char mock_nvm[NVM_SIZE];
nvm_image_t *nvm_image = (nvm_image_t*) mock_nvm;

// Initialize mock NVM module
// Every bit of uninitialized NVM data is set to 1, as in a real EEPROM
void nvm_mock_init(void) {
  memset(mock_nvm, 0xFF, NVM_SIZE);
  memcpy(mock_nvm, _nvm_image_ptr, sizeof(nvm_image_t));
}

void nvm_read(void *dest, const void *src, size_t size) {
  if (src < ((void*) nvm_image) || src + size - 1 > NVM_LIMIT)
    printf("Mock NVM error at function %s with dest=%p src=%p size=%d\n",
        __func__, dest, src, size);
  else memcpy(dest, src, size);
}

void nvm_write(void *dest, const void *src, size_t size) {
  if (dest < ((void*) nvm_image) || dest + size - 1 > NVM_LIMIT)
    printf("Mock NVM error at function %s with dest=%p src=%p size=%d\n",
        __func__, dest, src, size);
  else memcpy(dest, src, size);
}
