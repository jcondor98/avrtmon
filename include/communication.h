// avrtmon
// Communication handler - Head file
// Paolo Lucchesi - Fri 27 Sep 2019 07:29:48 PM CEST
#ifndef __COMMUNICATION_HANDLER_H
#define __COMMUNICATION_HANDLER_H
#include "packet.h"

// Type definition for a communication operation
typedef void (*com_operation_f)(const packet_t *rx_pack);
typedef com_operation_f* com_opmode_t;


// Initialize the communication module
void com_init(void);

// Perform a single "iteration" of the communication module activity
void com_handler(void);

// Switch communication opmode
void com_opmode_switch(const com_opmode_t opmode_new);

// Restore the communication opmode to the default one
void com_opmode_restore(void);

// Send a packet
void com_send(const packet_t*);

// Send an in-place crafted packet
void com_craft_and_send(packet_type_t type, const uint8_t *data,
    uint8_t data_size);

// Resend the last TX packet
void com_resend(void);

// Acknowledge a received packet
void com_ack(const packet_t*);

// Raise an error for a received packet
void com_err(const packet_t*);

#endif    // __COMMUNICATION_HANDLER_H
