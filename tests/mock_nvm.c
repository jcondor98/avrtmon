// avrtmon
// Mock NVM Implementation - Source file
#include <stdio.h>
#include <string.h>
#include <stddef.h> // offsetof macro
#include <assert.h>

#ifndef TEST
#error "Cannot use mock NVM module when not testing. You should '#define TEST'"
#endif

#include "temperature.h"
#include "nvm.h"

// Maximum usable address for the mock NVM image
#define NVM_LIMIT (((void*)nvm_image)+NVM_SIZE-1)


void nvm_read(void *dest, const void *src, size_t size) {
  if (src < ((void*) nvm_image) || src + size - 1 >= NVM_LIMIT)
    printf("Mock NVM error at function %s with dest=%p src=%p size=%d\n",
        __func__, dest, src, size);
  else memcpy(dest, src, size);
}

void nvm_write(void *dest, const void *src, size_t size) {
  if (dest < ((void*) nvm_image) || dest + size - 1 >= NVM_LIMIT)
    printf("Mock NVM error at function %s with dest=%p src=%p size=%d\n",
        __func__, dest, src, size);
  else memcpy(dest, src, size);
}
