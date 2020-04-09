// AVR Temperature Monitor -- Paolo Lucchesi
// Serial interface - Head file
// NOTE: Uses POSIX standard read() and write() for communication
#ifndef __SERIAL_MODULE_H
#define __SERIAL_MODULE_H
#include <unistd.h>
#include <pthread.h>
#include "packet.h"
#include "ringbuffer.h"

#define BAUD_RATE B57600

#if defined(DEBUG) && !defined(RX_BUF_SIZE)
#define RX_BUF_SIZE 512
#elif !defined(RX_BUF_SIZE)
#define RX_BUF_SIZE 64
#endif


// Serial context to make the module completely reentrant
typedef struct _serial_context_s {
  int dev_fd;
  struct {
    ringbuffer_t  *buffer;
    pthread_t     thread;
    unsigned char ongoing;
  } rx;
} serial_context_t;


// Open a serial device
// Return a pointer to an allocated and initialized context, or NULL on failure
serial_context_t *serial_open(const char *dev);

// Close a serial device
// Returns 0 on success, 1 if the descriptor given is not a tty device,
// -1 if the 'close' function fails
int serial_close(serial_context_t*);

// Get at most 'n' bytes from the serial port
// Return the number of bytes read
size_t serial_rx(serial_context_t*, void *buf, size_t size);

// Get a single character
#define serial_rx_getchar(ctx,dst) serial_rx(ctx,dst,1)

// Write data to the serial port with POSIX write
// Return the number of bytes effectively written or -1 on failure
ssize_t serial_tx(serial_context_t*, const void *buf, size_t size);

// Put a single character
#define serial_tx_putchar(ctx,src) serial_tx(ctx,src,1)

// Get the number of available data to read
size_t serial_rx_available(serial_context_t*);

// Race-protected getter for 'rx_ongoing' context variable
// Returns 0 on success, 1 if the serial context given is not valid
unsigned char serial_rx_ongoing(serial_context_t*);

// Flush the RX buffers (RX thread ringbuffer and kernel internal buffer)
void serial_rx_flush(serial_context_t*);

#endif  // __SERIAL_MODULE_H
