// AVR Temperature Monitor -- Paolo Lucchesi
// Multi-threaded, lock-free ringbuffer data structure - Head file
#ifndef __RINGBUFFER_STRUCT_H
#define __RINGBUFFER_STRUCT_H
#include <stddef.h>
#include <pthread.h>

enum RINGBUFFER_STATE_E { RINGBUFFER_NOT_FULL = 0, RINGBUFFER_FULL };

typedef struct _ringbuffer_s {
  unsigned char *base;
  pthread_mutex_t lock[1];
  size_t first;
  size_t last;
  size_t size;
  unsigned char full;
} ringbuffer_t;

// Initialize a ring buffer
ringbuffer_t *ringbuffer_new(size_t buf_size);

// Delete a ringbuffer
void ringbuffer_delete(ringbuffer_t*);

// Return the maximum number of items
size_t ringbuffer_size(ringbuffer_t*);

// Get the number of present items
size_t ringbuffer_used(ringbuffer_t*);

// Returns 1 if the buffer is empty, 0 if not or if it does not exist
unsigned char ringbuffer_isempty(ringbuffer_t*);

// Returns 1 if the buffer is full or if it does not exist, 0 if not
unsigned char ringbuffer_isfull(ringbuffer_t*);

// Pop an element
// Returns 0 on success, 1 otherwise
unsigned char ringbuffer_pop(ringbuffer_t*, unsigned char *dest);

// Push an element
// Returns 0 on success, 1 otherwise
unsigned char ringbuffer_push(ringbuffer_t*, unsigned char val);

// Flush (i.e. reset, empty) a ringbuffer
void ringbuffer_flush(ringbuffer_t*);

// Print the internal elements (without the raw buffer) of a ringbuffer
#if defined(TEST)
void ringbuffer_print(ringbuffer_t*);
#endif

#endif    // __RINGBUFFER_STRUCT_H
