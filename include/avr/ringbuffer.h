// avrtmon
// Ring Buffer data structure - Head file
// Paolo Lucchesi - Mon 30 Dec 2019 03:00:30 AM CET
#ifndef __RING_BUFFER_STRUCT_H
#define __RING_BUFFER_STRUCT_H
#include <stdint.h>

enum RINGBUFFER_STATE_E { RINGBUFFER_NOT_FULL = 0, RINGBUFFER_FULL };

typedef struct _ringbuffer_s {
  uint8_t *base;
  uint8_t first;
  uint8_t last;
  uint8_t size;
  uint8_t full;
} ringbuffer_t;

// Initialize a ring buffer
uint8_t ringbuffer_new(ringbuffer_t*, uint8_t *buf, uint8_t buf_size);

// Get the dimension of the buffer
uint8_t ringbuffer_size(ringbuffer_t*);

// Get the number of present items
uint8_t ringbuffer_used(ringbuffer_t*);

// Returns 1 if the buffer is empty, 0 if not or if it does not exist
// TODO: On inexistent buffer should return 1
uint8_t ringbuffer_isempty(ringbuffer_t*);

// Returns 1 if the buffer is full, 0 if not or if it does not exist
uint8_t ringbuffer_isfull(ringbuffer_t*);

// Pop an element
// Returns 0 on success, 1 otherwise
uint8_t ringbuffer_pop(ringbuffer_t*, uint8_t *dest);

// Push an element
// Returns 0 on success, 1 otherwise
uint8_t ringbuffer_push(ringbuffer_t*, uint8_t val);

// Flush (i.e. reset, empty) a ringbuffer
void ringbuffer_flush(ringbuffer_t*);

// Print the internal elements (without the raw buffer) of a ringbuffer
#if defined(TEST) || !defined(AVR)
void ringbuffer_print(ringbuffer_t *rb);
#endif

#endif    // __RING_BUFFER_STRUCT_H
