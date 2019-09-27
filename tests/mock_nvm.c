// avrtmon
// Mock NVM Implementation - Source file
#include <stdio.h>
#include <string.h>
#include <stddef.h> // offsetof macro
#include <assert.h>

#include "nvm_mock.h"
#include "temperature.h"

char _nvm[NVM_SIZE];
nvm_image_t *nvm = (nvm_image_t*) _nvm;

const void *NVM_LIMIT = ((void*) _nvm) + NVM_SIZE;

static inline void *redirect_addr_to_mock(const void *addr) {
  return addr < ((void*) nvm_image ) ? NULL :
    addr - ((void*) nvm_image) + ((void*) nvm);
}


// Perform a generic initialization of the NVM with a buffer given by the user
void mock_nvm_init() {
  assert(NVM_SIZE >= NVM_IMAGE_FULL_SIZE);
  memcpy(_nvm, nvm_image, NVM_IMAGE_FULL_SIZE);
}

void nvm_read(void *dest, const void *src, size_t size) {
  src = redirect_addr_to_mock(src);
  if (src < ((void*) nvm) || src + size - 1 >= NVM_LIMIT)
    printf("Mock NVM error at function %s with dest=%p src=%p size=%d\n",
        __func__, dest, src, size);
  else memcpy(dest, src, size);
}

void nvm_write(void *dest, const void *src, size_t size) {
  dest = redirect_addr_to_mock(dest);
  if (dest < ((void*) nvm) || dest + size - 1 >= NVM_LIMIT)
    printf("Mock NVM error at function %s with dest=%p src=%p size=%d\n",
        __func__, dest, src, size);
  else memcpy(dest, src, size);
}
