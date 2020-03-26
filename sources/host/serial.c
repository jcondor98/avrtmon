// avrtmon
// Serial interface - Source file
// Paolo Lucchesi - Wed 30 Oct 2019 11:55:02 PM CET
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>

#include "serial.h"
#include "debug.h"
 

#define ONE_MSEC 1000000
#define context_isvalid(ctx)\
  ((ctx) && (ctx)->dev_fd > 2 && isatty((ctx)->dev_fd))


// Auxiliary functions -- Source code at the bottom of this source file
static int _serial_rx_start(serial_context_t *ctx);
static void _serial_rx_stop(serial_context_t *ctx);


// Open a serial device
// On success, 0 is returned and 'ctx' is correctly initialized
// On failure, 1 is returned
// TODO: Free 'ctx' on error
serial_context_t *serial_open(const char *dev) {
  if (!dev || *dev == '\0') return NULL;
  serial_context_t *ctx = malloc(sizeof(serial_context_t));
  err_check(!ctx, NULL, "Unable to use the memory allocator");

  // Open the device file
  if ((ctx->dev_fd = open(dev, O_RDWR | O_NOCTTY)) < 0) {
    perror(__func__);
    free(ctx);
    return NULL;
  }

  if (!isatty(ctx->dev_fd)) {
    err_log("Device is not a serial TTY");
    close(ctx->dev_fd);
    free(ctx);
    return NULL;
  }

  // Setup serial device
  struct termios dev_io;
  tcgetattr(ctx->dev_fd, &dev_io);

  // Non-canonical communication with no hardware flow control, embedded parity,
  // double stop bit etc...
  dev_io.c_cflag &= ~(CRTSCTS | PARENB | CSTOPB | CSIZE);
  dev_io.c_iflag &= ~(IXON | IXOFF | IXANY);
  dev_io.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  dev_io.c_oflag = 0; // TODO: Make this portable

  dev_io.c_cflag    |= CS8 | CREAD | CLOCAL; // Read-enabled 8N1
  dev_io.c_cc[VMIN]  = 1;   // Never make read() calls indefinitely blocking
  dev_io.c_cc[VTIME] = 30;  // Wait at most 3 seconds for a read()
  //dev_io.c_lflag |= IGNBRK; // Ignore serial BREAK sequence


  // Set the serial baud rate
  cfsetospeed(&dev_io, BAUD_RATE);
  cfsetispeed(&dev_io, BAUD_RATE);

  // Apply changes made in 'dev_io' (immediately)
  tcsetattr(ctx->dev_fd, TCSANOW, &dev_io);

  // Initialize serial context
  ctx->rx.buffer = ringbuffer_new(RX_BUF_SIZE);   // TODO: Check errors and free 'ctx'
  pthread_mutex_init(ctx->rx.ongoing_lock, NULL); // TODO: Check errors and free 'ctx'
  ctx->rx.ongoing = 0;

  // Flush the kernel internal buffer
  // TODO: Only flush kernel internal buffer
  serial_rx_flush(ctx);

  _serial_rx_start(ctx); // Start receiving

  return ctx;
}


// Close a serial device
// Returns 0 on success, 1 if the serial context given is not valid
int serial_close(serial_context_t *ctx) {
  if (!context_isvalid(ctx)) return 1;

  // Stop and destroy rx thread
  if (ctx->rx.ongoing) {
    _serial_rx_stop(ctx);
    if (pthread_join(ctx->rx.thread, NULL) != 0)
      err_log("Unable to join RX thread");
  }

  // Close the serial port file descriptor - Don't check errors
  if (close(ctx->dev_fd) != 0)
    err_log("Unable to close device descriptor");
  ctx->dev_fd = -1; // This serial context is not valid anymore

  if (pthread_mutex_destroy(ctx->rx.ongoing_lock) != 0) // Destroy RX mutexes
    err_log("Unable to destroy RX ongoing mutex");

  return 0;
}


// Get at most 'n' bytes from the serial port
// Return the number of bytes read
size_t serial_rx(serial_context_t *ctx, void *dest, size_t size) {
  if (!context_isvalid(ctx) || !dest || !serial_rx_ongoing(ctx))
    return 0;

  size_t n;
  for (n=0; n < size; ++n)
    if (ringbuffer_pop(ctx->rx.buffer, dest + n) != 0)
      break;

  return n;
}


// Race-protected getter for 'rx_ongoing' context variable
// Returns 0 if not receiving, 1 otherwise
unsigned char serial_rx_ongoing(serial_context_t *ctx) {
  pthread_mutex_lock(ctx->rx.ongoing_lock);
  int ongoing = ctx->rx.ongoing;
  pthread_mutex_unlock(ctx->rx.ongoing_lock);

  return ongoing;
}


// Get the number of available data to read
size_t serial_rx_available(serial_context_t *ctx) {
  if (context_isvalid(ctx) && ctx->rx.ongoing)
    return ringbuffer_used(ctx->rx.buffer);
  else return 0;
}

// Flush the RX buffers (RX thread ringbuffer and kernel internal buffer)
void serial_rx_flush(serial_context_t *ctx) {
  if (!ctx) return;
  ringbuffer_flush(ctx->rx.buffer); // Flush the RX thread ringbuffer

  // Flush the kernel internal buffer for the file descriptor
  // The 'sleep' is for a bug in the linux kernel that probably won't be fixed
  // https://bugzilla.kernel.org/show_bug.cgi?id=5730
  static const struct timespec flush_sleep_val = { 0, ONE_MSEC };
  nanosleep(&flush_sleep_val, NULL);
  tcflush(ctx->dev_fd, TCIOFLUSH); // TODO: TCOFLUSH?
}


// Write data to the serial port with POSIX write
// Return the number of bytes effectively written or -1 on failure
ssize_t serial_tx(serial_context_t *ctx, const void *buf, size_t size) {
  ssize_t written = write(ctx->dev_fd, buf, size);
  err_check_perror(written < 0, -1);

  // Wait for the data to be physically sent and give the AVR a while
  static const struct timespec write_sleep_val = { 0, ONE_MSEC };
  tcdrain(ctx->dev_fd);
  nanosleep(&write_sleep_val, NULL);

  return written;
}


// [AUX] Tell the RX thread to stop receiving data (but do not destroy it)
static void _serial_rx_stop(serial_context_t *ctx) {
  pthread_mutex_lock(ctx->rx.ongoing_lock);
  ctx->rx.ongoing = 0;
  pthread_mutex_unlock(ctx->rx.ongoing_lock);
}


// [AUX] RX thread task
// Constantly read data and store it in the RX ringbuffer
#define RX_INTER_BUF_SIZE 128
static void *_serial_rx_task(void *arg) {
  serial_context_t *ctx = arg;
  unsigned char rx_inter_buf[RX_INTER_BUF_SIZE];

  debug err_log("RX now ongoing");
  while (serial_rx_ongoing(ctx)) {
    //debug printf("[RX] New iteration\n");
    ssize_t received = read(ctx->dev_fd, rx_inter_buf, RX_INTER_BUF_SIZE);
    if (received < 0) perror(__func__);
    else for (size_t i=0; i < received; ++i) {
      unsigned char c = rx_inter_buf[i];
      ringbuffer_push(ctx->rx.buffer, c);
      //debug ringbuffer_print(ctx->rx.buffer);
      debug err_log("Received byte: 0x%hhx", c);
    }
  }

  pthread_exit(NULL);
}


// [AUX] Start receiving data in a separated (POSIX) thread
// Returns 0 if the thread is started and running, 1 otherwise
static int _serial_rx_start(serial_context_t *ctx) {
  ctx->rx.ongoing = 1;
  if (pthread_create(&ctx->rx.thread, NULL, _serial_rx_task, ctx) == 0)
    return 0;

  // Error on pthread creation
  ctx->rx.ongoing = 0;
  err_log("Unable to start new thread");
  return 1;
}
