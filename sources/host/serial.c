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
 
#define BAUD_RATE B57600


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
