// avrtmon
// Serial interface - Head file
// Paolo Lucchesi - Wed 30 Oct 2019 11:55:38 PM CET
#ifndef __SERIAL_INTERFACE_H
#define __SERIAL_INTERFACE_H

// Open a serial device
// Returns an open file descriptor of the serial device, or -1 on error
int serial_open(const char *dev);

// Close a serial device
// Returns 0 on success, 1 if the descriptor given is not a tty device,
// -1 if the 'close' function fails
int serial_close(int dev_fd);

#endif    // __SERIAL_INTERFACE_H
