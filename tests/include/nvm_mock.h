// avrtmon
// Mock NVM Implementation - Head file
// Paolo Lucchesi - Thu 22 Aug 2019 04:22:50 PM CEST
#ifndef __NVM_INTERFACE_MOCK_H
#define __NVM_INTERFACE_MOCK_H
#include "nvm.h"

// Undefine all the stuff conflicting with the "true" NVM interface
#undef NVMMEM
#undef NVM_LIMIT
#undef nvm_read
#undef nvm_write
#undef nvm_update
#undef nvm_ongoing
#undef nvm_busy_wait


// EEMEM expands to nothing
#define NVMMEM

// NVM_LIMIT is a real pointer now
const void *NVM_LIMIT;

// Perform a generic initialization of the NVM with a buffer given by the user
void mock_nvm_init(void);

void nvm_read(void *dest, const void *src, size_t size);
void nvm_write(void *dest, const void *src, size_t size);

#define nvm_update nvm_write
#define nvm_ongoing() 0;
#define nvm_busy_wait() do {} while (0)

// Expose the NVM for testing
nvm_image_t *nvm;

#endif    // __NVM_INTERFACE_H
