// avrtmon
// Mock NVM Implementation - Head file
// Paolo Lucchesi - Thu 22 Aug 2019 04:22:50 PM CEST
#ifndef __NVM_INTERFACE_MOCK_H
#define __NVM_INTERFACE_MOCK_H

#ifndef __NVM_INTERFACE_H
#error "Cannot use mock NVM interface without base NVM module"
#endif

// EEMEM expands to nothing
#define NVMMEM

void _nvm_read(void *dest, const void *src, size_t size);
void _nvm_write(void *dest, const void *src, size_t size);

#define nvm_read _nvm_read
#define nvm_write _nvm_write
#define nvm_update _nvm_write
#define nvm_ongoing() 0;
#define nvm_busy_wait() do {} while (0)

#endif    // __NVM_INTERFACE_MOCK_H
