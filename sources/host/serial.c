// AVR Temperature Monitor -- Paolo Lucchesi
// Serial interface - Source file
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

// [AUX] RX thread task
static void *_serial_rx_task(void *arg);


// Open a serial device
// Return a pointer to an allocated and initialized context, or NULL on failure
serial_context_t *serial_open(const char *dev) {
  if (!dev || *dev == '\0') return NULL;
  serial_context_t *ctx = malloc(sizeof(serial_context_t));
  err_check(!ctx, NULL, "Unable to use the memory allocator");

  ctx->rx.buffer = ringbuffer_new(RX_BUF_SIZE);
  if (!ctx->rx.buffer) {
    free(ctx);
    error(NULL, "Unable to use the memory allocator");
  }

  // Open the device file
  if ((ctx->dev_fd = open(dev, O_RDWR | O_NOCTTY)) < 0) {
    perror(__func__);
    free(ctx->rx.buffer);
    free(ctx);
    return NULL;
  }

  if (!isatty(ctx->dev_fd)) {
    close(ctx->dev_fd);
    free(ctx->rx.buffer);
    free(ctx);
    error(NULL, "Device is not a serial TTY");
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
  dev_io.c_cc[VMIN]  = 1;   // Read at least 1 character
  dev_io.c_cc[VTIME] = 30;  // Wait at most 3 seconds for a read()


  // Set the serial baud rate
  cfsetospeed(&dev_io, BAUD_RATE);
  cfsetispeed(&dev_io, BAUD_RATE);

  // Apply changes made in 'dev_io' and flush the RX kernel buffer
  tcsetattr(ctx->dev_fd, TCSANOW, &dev_io);
  tcflush(ctx->dev_fd, TCIFLUSH);

  // Start RX thread
  ctx->rx.ongoing = 1;
  if (pthread_create(&ctx->rx.thread, NULL, _serial_rx_task, ctx) != 0) {
    close(ctx->dev_fd);
    free(ctx->rx.buffer);
    free(ctx);
    error(NULL, "Could not start RX thread");
  }

  return ctx;
}


// Close a serial device
// Returns 0 on success, 1 if the serial context given is not valid
int serial_close(serial_context_t *ctx) {
  if (!context_isvalid(ctx)) return 1;

  // Stop and destroy rx thread
  if (ctx->rx.ongoing) {
    pthread_cancel(ctx->rx.thread);
    if (pthread_join(ctx->rx.thread, NULL) != 0)
      err_log("Unable to join RX thread");
  }

  // Close the serial port file descriptor
  if (close(ctx->dev_fd) != 0)
    err_log("Unable to close device descriptor");

  // Destroy the RX ringbuffer
  ringbuffer_delete(ctx->rx.buffer);

  free(ctx);
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


// Getter for 'rx_ongoing' context variable
// Returns 0 if not receiving, 1 otherwise
unsigned char serial_rx_ongoing(serial_context_t *ctx) {
  return context_isvalid(ctx) ? ctx->rx.ongoing : 0;
}

// Get the number of available data to read
size_t serial_rx_available(serial_context_t *ctx) {
  return (context_isvalid(ctx)) ? ringbuffer_used(ctx->rx.buffer) : 0;
}


// Flush the RX buffers (RX thread ringbuffer and kernel internal buffer)
void serial_rx_flush(serial_context_t *ctx) {
  if (!context_isvalid(ctx)) return;
  ringbuffer_flush(ctx->rx.buffer); // Flush the RX thread ringbuffer

  // Flush the kernel internal buffer for the file descriptor
  // The 'sleep' is for a bug in the linux kernel that probably won't be fixed
  // https://bugzilla.kernel.org/show_bug.cgi?id=5730
  static const struct timespec flush_sleep_val = { 0, ONE_MSEC };
  nanosleep(&flush_sleep_val, NULL);
  tcflush(ctx->dev_fd, TCIFLUSH);
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



// [AUX] RX thread task
// Constantly read data and store it in the RX ringbuffer
#define RX_INTER_BUF_SIZE 32
static void *_serial_rx_task(void *arg) {
  debug err_log("RX now ongoing");

  serial_context_t *ctx = arg;
  unsigned char rx_inter_buf[RX_INTER_BUF_SIZE];

  while (1) {
    //debug printf("[RX] New iteration\n");

    // Interruptable read
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    ssize_t received = read(ctx->dev_fd, rx_inter_buf, RX_INTER_BUF_SIZE);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    if (received < 0) perror(__func__);
    else for (size_t i=0; i < received; ++i) { // TODO: Push in bulk
      unsigned char c = rx_inter_buf[i];
      ringbuffer_push(ctx->rx.buffer, c);
      //debug printf("[RX] Received byte: 0x%hhx\n", c);
    }
  }

  pthread_exit(NULL); // Never reached
}
