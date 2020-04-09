// AVR Temperature Monitor -- Paolo Lucchesi
// Communication handler - Head file
#ifndef __COMMUNICATION_MODULE_H
#define __COMMUNICATION_MODULE_H
#include "packet.h"

// Number of maximum attempts to send/receive a single packet
#define MAXIMUM_SEND_ATTEMPTS 3
#define MAXIMUM_RECV_ATTEMPTS 3

// Precise error state codes for receiving/sending errors
typedef enum ERR_CODE_E {
  E_SUCCESS = 0, E_TIMEOUT_ELAPSED, E_CORRUPTED_HEADER,
  E_CORRUPTED_CHECKSUM, E_ID_MISMATCH
} err_code_t;

// Type definition for a communication operation
typedef uint8_t (*com_operation_f)(const packet_t *rx_pack);
typedef com_operation_f* com_opmode_t;


// Initialize the communication module
void communication_init(void);

// Get an incoming packet if data is available on the serial port (blocking)
// Returns 0 on success, 1 on failure
uint8_t communication_recv(packet_t*);

// Send a packet (blocking)
// Returns 0 if the packet is sent correctly, 1 otherwise
uint8_t communication_send(const packet_t*);

// Send an in-place crafted packet
// Returns 0 if the packet is sent correctly, 1 otherwise
uint8_t communication_craft_and_send(packet_type_t type, const uint8_t *data,
    uint8_t data_size);

// Perform a single "iteration" of the communication module activity
// Returns 0 if no significant action was performed, non-zero otherwise
uint8_t communication_handler(void);

// Switch communication opmode
void communication_opmode_switch(const com_opmode_t opmode_new);

// Restore the communication opmode to the default one
void communication_opmode_restore(void);

// Send an in-place crafted packet
// Returns 0 if the packet is sent correctly, 1 otherwise
void com_craft_and_send(packet_type_t type, const uint8_t *data,
    uint8_t data_size);

#endif  // __COMMUNICATION_MODULE_H
