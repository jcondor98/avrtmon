// avrtmon
// Communication layer (host-side) - Head file
// Paolo Lucchesi - Tue 25 Feb 2020 12:10:36 PM CET
#ifndef __COMMUNICATION_LAYER_H
#define __COMMUNICATION_LAYER_H
#include "packet.h"
#include "serial.h"
#include "command.h"

/* TODO: Keep or remove?
// Resolution of the RTO timer in millisecond
#define RTO_RESOLUTION 6

// Timing Parameters (in RTO_RESOLUTION units)
#define RTO_DELIVERY_TIME 4 // Time to (only) deliver a packet
#define RTO_PROCESS_TIME  1 // Time to process a packet (received or to be sent)
*/

// RTO value in milliseconds
#define RTO_VALUE_MSEC 500

// Number of maximum attempts to send/receive a single packet
#define MAXIMUM_SEND_ATTEMPTS 3
#define MAXIMUM_RECV_ATTEMPTS 3

// Precise error state codes for receiving/sending errors
typedef enum ERR_CODE_E {
  E_SUCCESS = 0, E_TIMEOUT_ELAPSED, E_CORRUPTED_HEADER, E_CORRUPTED_CHECKSUM, E_ID_MISMATCH
} err_code_t;


// Initialize the communication module
int communication_init(void);

// Cleanup for the communication module
void communication_cleanup(void);

// Estabilish a connection, sending a handshake (HND) packet
// Return 0 on success, 1 on failure
int communication_connect(serial_context_t *ctx);

// Send a packet
// Returns 0 on success, 1 on failure
// Never use for HND, ACK or ERR packet types
int communication_send(serial_context_t *ctx, const packet_t *p);

// Receive a packet
// Returns 0 on success, 1 on failure
// Never use for HND, ACK or ERR packet types
int communication_recv(serial_context_t *ctx, packet_t *p);

// Craft a packet in-place and send it
// Returns 0 on success, 1 on failure
// Never use for HND, ACK or ERR packet types
int communication_craft_and_send(serial_context_t *ctx, unsigned char type,
    const unsigned char *data, unsigned char data_size);

// Craft a command packet in-place and send it
// Return 0 if the packet was sent correctly, 1 otherwise
int communication_cmd(serial_context_t *ctx, command_id_t cmd,
    const void *arg, unsigned arg_size);

#endif    // __COMMUNICATION_LAYER_H
