// avrtmon
// Mock NVM Implementation - Head file
// Paolo Lucchesi - Thu 22 Aug 2019 04:22:50 PM CEST
#ifndef __NVM_INTERFACE_H
#define __NVM_INTERFACE_H

#define NVM_SIZE 16

void mock_nvm_init_for_temperature_db(void);

void nvm_read(void *dest, const void *src, size_t size);
void nvm_write(void *dest, const void *src, size_t size);

#define nvm_update nvm_write
#define nvm_ongoing() 0;
#define nvm_busy_wait() do {} while (0)

#endif    // __NVM_INTERFACE_H
