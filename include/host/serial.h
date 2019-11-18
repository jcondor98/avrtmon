// avrtmon
// Serial interface - Head file
// Paolo Lucchesi - Wed 30 Oct 2019 11:55:38 PM CET
#ifndef __SERIAL_INTERFACE_H
#define __SERIAL_INTERFACE_H
#include "packet.h"
#include "command.h"

// Open a serial device
// Returns an open file descriptor of the serial device, or -1 on error
int serial_open(const char *dev);

// Close a serial device
// Returns 0 on success, 1 if the descriptor given is not a tty device,
// -1 if the 'close' function fails
int serial_close(int dev_fd);

// Craft on-the-fly and send a packet
void serial_send(const packet_t *packet);

// Craft on-the-fly and send a packet
// Returns 0 on success, 1 otherwise
int serial_craft_and_send(packet_type_t type, const void *data,
    unsigned data_size, int dev_fd);

// Craft on-the-fly and send a command packet (handier)
// Returns 0 on success, 1 otherwise
int serial_cmd(command_id_t cmd, const void *arg, unsigned arg_size, int dev_fd);

// Receive a packet from the tmon, storing it into dest
// Returns 0 on success, 1 otherwise
int serial_recv(int dev_fd, packet_t *dest);

#endif    // __SERIAL_INTERFACE_H
