// avrtmon
// Multi-threaded ringbuffer data structure - Source file
// Paolo Lucchesi - Mon 30 Dec 2019 03:35:41 AM CET
#include "ringbuffer.h" // Also includes pthread library
#include <stdlib.h>

#if defined(TEST) || !defined(AVR)
#include <stdio.h>
#include <string.h>
#endif


// [AUX] Get the real index of an item given its virtual address
static inline size_t real_idx(const ringbuffer_t *rb, size_t virt) {
  return (virt + rb->first) % rb->size;
}

// [AUX] Get the virtual index of an item given its real address
static inline size_t virt_idx(const ringbuffer_t *rb, size_t real) {
  return (real + rb->size - rb->first) % rb->size;
}

// [AUX] Get the shifted (i.e. incremented) index for a virtual one
static inline size_t shifted_idx(const ringbuffer_t *rb, size_t idx) {
  return (idx + 1) % rb->size;
}

// Return 1 if a buffer is free/full, 0 otherwise
static inline unsigned char _isfull(ringbuffer_t *rb) { return rb->full; }
static inline unsigned char _isempty(ringbuffer_t *rb) {
  return (rb->first == rb->last && !rb->full) ? 1 : 0;
}

// Lock/Unlock mutex
static inline void lock(ringbuffer_t *rb)   { pthread_mutex_lock(rb->lock); }
static inline void unlock(ringbuffer_t *rb) { pthread_mutex_unlock(rb->lock); }


// Initialize a ring buffer
ringbuffer_t *ringbuffer_new(size_t size) {
  if (size < 2) return NULL;

  ringbuffer_t *rb = malloc(sizeof(ringbuffer_t));
  if (!rb) return NULL;

  rb->base = malloc(size + 1);
  if (!rb->base) {
    free(rb);
    return NULL;
  }

  if (pthread_mutex_init(rb->lock, NULL) != 0) {
    free(rb->base);
    free(rb);
    return NULL;
  }

  rb->first = 0;
  rb->last = 0;
  rb->size = size;
  rb->full = 0;

  return rb;
}


// Delete a ringbuffer
void ringbuffer_delete(ringbuffer_t *rb) {
  if (!rb) return;
  pthread_mutex_destroy(rb->lock); // Ignore errors
  free(rb->base);
  free(rb);
}


// Return the maximum number of items
// 'size' will be constant and untouched, so do not lock on this operation
size_t ringbuffer_size(ringbuffer_t *rb) { return rb ? rb->size : 0; }

// Get the number of present items
size_t ringbuffer_used(ringbuffer_t *rb) {
  if (!rb) return 0;
  size_t ret;

  lock(rb);
  if (_isfull(rb)) ret = rb->size;
  else ret = virt_idx(rb, rb->last);
  unlock(rb);

  return ret;
}


// Returns 1 if the buffer is empty, 0 if not or if it does not exist
unsigned char ringbuffer_isempty(ringbuffer_t *rb) {
  if (!rb) return 1;

  lock(rb);
  unsigned char ret = _isempty(rb);
  unlock(rb);

  return ret;
}


// Returns 1 if the buffer is full or if it does not exist, 0 if not
unsigned char ringbuffer_isfull(ringbuffer_t *rb) {
  if (!rb) return 1;

  lock(rb);
  unsigned char ret = _isfull(rb);
  unlock(rb);

  return ret;
}


// Pop an element
// Returns 0 on success, 1 otherwise
unsigned char ringbuffer_pop(ringbuffer_t *rb, unsigned char *dest) {
  if (!rb) return 1;
  unsigned char ret;

  lock(rb);
  if (_isempty(rb)) ret = 1;
  else {
    *dest = rb->base[rb->first];
    rb->first = shifted_idx(rb, rb->first);
    rb->full = RINGBUFFER_NOT_FULL;
    ret = 0;
  }
  unlock(rb);

  return ret;
}


// Push an element
unsigned char ringbuffer_push(ringbuffer_t *rb, unsigned char val) {
  if (!rb) return 1;
  unsigned char ret;

  lock(rb);
  if (_isfull(rb)) ret = 1;
  else {
    rb->base[rb->last] = val;
    rb->last = shifted_idx(rb, rb->last);
    if (rb->first == rb->last)
      rb->full = RINGBUFFER_FULL;
    ret = 0;
  }
  unlock(rb);

  return ret;
}


// Flush (i.e. reset, empty) a ringbuffer
void ringbuffer_flush(ringbuffer_t *rb) {
  if (!rb) return;
  lock(rb);
  rb->first = rb->last;
  rb->full = RINGBUFFER_NOT_FULL;
  unlock(rb);
}


// Print the internal elements (without the raw buffer) of a ringbuffer
#if defined(TEST)
void ringbuffer_print(ringbuffer_t *rb) {
  if (!rb) return;

  // Make a local copy of the ringbuffer to release it ASAP
  lock(rb);
  ringbuffer_t _rb;
  memcpy(&_rb, rb, sizeof(ringbuffer_t));
  unlock(rb);

  printf("Printing ringbuffer\n"
      "base:  %p\n"
      "first: %zu\n"
      "last:  %zu\n"
      "size:  %zu\n\n",
      _rb.base, _rb.first, _rb.last, _rb.size);
}
#endif
