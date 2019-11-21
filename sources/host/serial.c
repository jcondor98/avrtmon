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

#include "host/serial.h"
#include "packet.h"
 
#define BAUD_RATE B57600


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
  dev_io.c_cc[VMIN]  = 1;
  dev_io.c_cc[VTIME] = 5;

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
    return close(dev_fd);
  else return 1;
}


// Craft on-the-fly and send a packet
// Returns 0 on success, 1 otherwise
int serial_craft_and_send(packet_type_t type, const void *data,
    unsigned data_size, int dev_fd) {
  if (type >= PACKET_TYPE_COUNT || (!data && data_size) ||
      data_size > PACKET_DATA_MAX_SIZE || dev_fd < 0)
    return 1;

  packet_t p;
  switch (type) {
    // HND is a single-byte packet
    case PACKET_TYPE_HND:
      packet_craft(type, NULL, 0, &p);
      if (_write(dev_fd, (void*) &p, 1) != 0)
        return 1;
      break;

    // ACK and ERR are not supported in this context
    case PACKET_TYPE_ACK:
    case PACKET_TYPE_ERR:
      return 1;

    // Generic packet (i.e. DAT, CMD or CTR)
    default:
      packet_craft(type, data, data_size, &p);
      if (_write(dev_fd, (void*) &p, p.size) != 0)
        return 1;
  }

  return 0;
}


// Receive a packet from the tmon, storing it into dest
// Returns 0 on success, 1 otherwise
int serial_recv(int dev_fd, packet_t *dest) {
  if (!dest || dev_fd < 0) return 1;

  // Get first byte of the header
  _read(dev_fd, (void*) dest, 1);

  // If the packet is not a single-byte one, get the size
  switch (dest->type) {
    case PACKET_TYPE_HND:
    case PACKET_TYPE_ACK:
    case PACKET_TYPE_ERR:
      return 0;
    default:
      // Trick here: take the address of the second byte of the header
      _read(dev_fd, ((unsigned char*) dest) + 1, 1);
      for (int i=0; i < dest->size - PACKET_HEADER_SIZE; ++i)
        _read(dev_fd, &dest->data[i], 1);
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
    int ret;
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
  for (size_t i=0; i < nbytes; ++i) {
    int ret;
    while ((ret = read(fd, buf + i, 1)) <= 0)
      if (ret < 0 && errno != EINTR) {
        perror(__func__);
        return 1;
      }
  }
  return 0;
}