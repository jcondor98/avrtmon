// avrtmon
// Serial interface - Source file
// Paolo Lucchesi - Wed 30 Oct 2019 11:55:02 PM CET
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
//#include <sys/stat.h>
#include <errno.h>
#include <assert.h>

#include "host/serial.h"
#include "host/debug.h"
#include "packet.h"
 
#define BAUD_RATE B57600
#define CONSECUTIVE_ERRORS_THRESHOLD 10


// [AUX] Write a buffer to the serial port, one byte at a time (blocking)
int _write(int fd, unsigned char *buf, size_t nbytes);

// [AUX] Read from the serial port, one byte at a time (blocking)
int _read(int fd, unsigned char *buf, size_t nbytes);


// Open a serial device
// Returns an open file descriptor of the serial device, or -1 on error
int serial_open(const char *dev) {
  if (!dev || *dev == '\0') return -1;

  // Termios struct and descriptor for the device to open
  struct termios dev_io = { 0 };
  int dev_fd;

  // Setup serial device
  dev_io.c_cflag     = CS8 | CREAD | CLOCAL;
  dev_io.c_cc[VMIN]  = 0;   // Never make read() calls indefinitely blocking
  dev_io.c_cc[VTIME] = 30;  // Wait at most 3 seconds between characters

  // Open the device file
  dev_fd = open(dev, O_RDWR);
  if (dev_fd < 0) return -1;
  if (!isatty(dev_fd)) {
    close(dev_fd);
    return -1;
  }

  // Set the serial baud rate
  cfsetospeed(&dev_io, BAUD_RATE);
  cfsetispeed(&dev_io, BAUD_RATE);

  // Apply changes made in 'dev_io' (immediately)
  tcsetattr(dev_fd, TCSANOW, &dev_io);
  return dev_fd;
}


// Close a serial device
// Returns 0 on success, 1 if the descriptor given is not a tty device,
// -1 if the 'close' function fails
int serial_close(int dev_fd) {
  if (dev_fd < 3 || !isatty(dev_fd))  // fd < 3 => Exclude stdin/stdout/stderr
    return 1;
  else return close(dev_fd);
}


// Craft on-the-fly and send a packet
// Returns 0 on success, 1 otherwise
int serial_craft_and_send(packet_type_t type, const void *data,
    unsigned data_size, int dev_fd) {
  if (type >= PACKET_TYPE_COUNT || (!data && data_size) ||
      data_size > PACKET_DATA_MAX_SIZE || dev_fd < 0)
    return 1;

  // Handle packet creation given its type
  packet_t p;
  switch (type) {
    // ACK and ERR are not supported in this context
    case PACKET_TYPE_ACK:
    case PACKET_TYPE_ERR:
      return 1;

    // Generic packet (i.e. DAT, CMD or CTR)
    default:
      packet_craft(type, data, data_size, &p);
      break;
  }

  debug {
    puts("Crafted a new packet to send");
    packet_print(&p);
  }

  // Continously send the packet until an ACK response is received
  for (unsigned i=0; i <= CONSECUTIVE_ERRORS_THRESHOLD; ++i) {
    packet_t res;  // Response packet

    // Send the packet, exiting on error
    if (_write(dev_fd, (void*) &p, p.size) != 0)
      return 1;
    debug puts("Packet was sent");

    // Receive the response
    serial_recv(dev_fd, &res);
    debug {
      printf("Response received\n");
      packet_print(&res);
    }
    if (res.type == PACKET_TYPE_ACK) return 0;
    else if (res.type != PACKET_TYPE_ERR) // Error: unexpected packet type
      return 1;
  }

  fputs("Error: Too many consecutive errors while receiving package\n", stderr);
  return 1;
}


// Receive a packet from the tmon, storing it into dest
// Returns 0 on success, 1 otherwise
// TODO: Add error checking
int serial_recv(int dev_fd, packet_t *dest) {
  if (!dest || dev_fd < 0) return 1;

  // Get first byte of the header
  _read(dev_fd, (void*) dest, PACKET_HEADER_SIZE);

  // If the packet brings data, get it
  uint8_t dest_data_size = dest->size - PACKET_HEADER_SIZE;
  if (packet_brings_data(dest) && dest_data_size)
    _read(dev_fd, dest->data, dest_data_size);

  debug {
    printf("\nReceived packet\n");
    packet_print(dest);
  }

  return 0;
}

// Craft on-the-fly and send a command packet (handier)
// Returns 0 on success, 1 otherwise
int serial_cmd(command_id_t id, const void *arg, unsigned arg_size, int dev_fd) {
  if (id >= COMMAND_COUNT || dev_fd < 0 || arg_size > PACKET_DATA_MAX_SIZE -
      sizeof(command_id_t) || (!arg && arg_size))
    return 1;

  char _payload[PACKET_DATA_MAX_SIZE];
  command_payload_t *payload = (command_payload_t*) _payload;

  payload->id = id;
  if (arg) memcpy(payload->arg, arg, arg_size);

  return serial_craft_and_send(PACKET_TYPE_CMD, payload,
      sizeof(command_id_t) + arg_size, dev_fd);
}



// [AUX] Write a buffer to the serial port, one byte at a time (blocking)
int _write(int fd, unsigned char *buf, size_t nbytes) {
  for (size_t i=0; i < nbytes; ++i) {
    ssize_t ret;
    while ((ret = write(fd, buf + i, 1)) <= 0)
      if (ret < 0 && errno != EINTR) {
        perror(__func__);
        return 1;
      }
  }
  return 0;
}

// [AUX] Read from the serial port, one byte at a time (blocking)
int _read(int fd, unsigned char *buf, size_t nbytes) {
  for (ssize_t bytes_read = 0; bytes_read < nbytes; ++bytes_read) {
    ssize_t ret = read(fd, buf, nbytes) == nbytes ? 0 : 1;
    if (ret < 0)
      switch (errno) {
        case EINTR: continue;
        case EAGAIN:
          fputs("Error: timeout was reached for reading data", stderr);
          return 1;
        default:
          perror(__func__);
          return 1;
      }
    else bytes_read += ret;
  }
  return 0;
}
