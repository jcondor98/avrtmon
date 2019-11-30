// avrtmon
// Communication handler - Source file
// Paolo Lucchesi - Fri 27 Sep 2019 07:28:26 PM CEST
#include <string.h>  // memcpy
#include "communication.h"
#include "command.h"
#include "packet.h"
#include "serial.h"


// Buffers for serial RX and TX
// Raw buffers are treated as packets, pointers to them are generic
static volatile packet_t rx_pack;
static packet_t tx_pack;
static volatile void *rx_buf = &rx_pack;
static void *tx_buf = &tx_pack;

// Default opmode
static com_operation_f opmode_default[];

// Opmode currently in use
static com_opmode_t opmode = opmode_default;


// Busy wait for each byte of a packet
// It is assumed that at least a byte of the packet is already available
// Returns 0 if the packet is sane, 1 if it is corrupted
uint8_t _rx_pack_busy_wait(void) {
  uint8_t available = serial_rx_available();
  uint8_t type = rx_pack.type;

  // Early check against bad packet types
  if (type > PACKET_TYPE_COUNT)
    return 1;

  // If the packet is single-byte, return immediately
  if (type == PACKET_TYPE_HND || type == PACKET_TYPE_ERR ||
      type == PACKET_TYPE_ACK)
    return 0;


  // Wait for the entire header to arrive
  while (available < 2)
    available = serial_rx_available();

  // Check the packet header integrity
  if (packet_check_header((void*) rx_buf) != 0) {
   com_err((const packet_t*) rx_buf);
   return 1;
  }

  // Wait for all the bytes to be received
  uint8_t size = rx_pack.size;
  while (available < size)
    available = serial_rx_available();

  // Check the packet against its CRC
  if (packet_check_crc((void*) rx_buf) != 0)
    return 1;

  return 0; // Success
}


// Communication handler
void com_handler(void) {
  const uint8_t available = serial_rx_available();

  // If no data is available, return immediately
  if (!available) return;

  // If the packet is corrupted, send an error
  if (_rx_pack_busy_wait() != 0)
    com_err((const packet_t*) rx_buf);

  // If the packet is sane, process it
  else {
    // Execute the operation for the packet to process -- If the packet type is
    // not supported by the opmode in use, fallback to the default one
    packet_type_t type = rx_pack.type;
    if (type != PACKET_TYPE_ACK && type != PACKET_TYPE_ERR)
      com_ack((const packet_t*) rx_buf);
    if (opmode[type])
      opmode[type]((const packet_t*) rx_buf);
    else
      opmode_default[type]((const packet_t*) rx_buf);
  }

  // Before returning, start waiting for another packet
  serial_rx(rx_buf, sizeof(packet_t));
}


// Switch communication opmode
void com_opmode_switch(const com_opmode_t opmode_new) {
  if (opmode_new) opmode = opmode_new;
}

// Restore the communication opmode to the default one
void com_opmode_restore(void) {
  opmode = opmode_default;
}

// Send the packet that is currently loaded in 'tx_pack'
static inline uint8_t _com_send(void) {
  uint8_t size;
  if (tx_pack.type == PACKET_TYPE_ACK || tx_pack.type == PACKET_TYPE_ERR)
    size = 1;
  else size = tx_pack.size;
  return serial_tx(tx_buf, size);
}

// Send a packet
void com_send(const packet_t *pack) {
  if (!pack) return;
  memcpy(tx_buf, pack, sizeof(packet_t));
  _com_send();
}

// Resend the last TX packet
void com_resend(void) { _com_send(); }

// Send an in-place crafted packet
void com_craft_and_send(packet_type_t type, const uint8_t *data,
    uint8_t data_size) {
  if (packet_craft(type, data, data_size, &tx_pack) == 0)
    _com_send();
}

// Acknowledge a received packet
void com_ack(const packet_t *pack) {
  tx_pack.id = pack->id;
  tx_pack.type = PACKET_TYPE_ACK;
  _com_send();
}

// Raise an error for a received packet
void com_err(const packet_t *pack) {
  tx_pack.id = pack->id;
  tx_pack.type = PACKET_TYPE_ERR;
  _com_send();  // Just resend the just sent packet
}

// Initialize the communication module
void com_init(void) {
  serial_init();
  serial_rx(rx_buf, sizeof(packet_t));
}



// Operation for PACKET_TYPE_HND -- Reset the communication environment
static void _op_hnd(const packet_t *rx_pack) {
  com_opmode_restore();
  serial_rx_reset();
  serial_tx_reset();
}

// Operation for PACKET_TYPE_ERR -- Resend the packet
static void _op_err(const packet_t *rx_pack) {
  com_resend();
}

// Operation for PACKET_TYPE_CMD -- Change operation table and execute command
static void _op_cmd(const packet_t *rx_pack) {
  const command_payload_t *payload = (const command_payload_t*) rx_pack->data;
  command_exec(payload->id, (const void*) payload->arg);
}

// For ACK, CTR and DAT do nothing
static void _op_ack(const packet_t *rx_pack) {}
static void _op_ctr(const packet_t *rx_pack) {}
static void _op_dat(const packet_t *rx_pack) {}

// The opmode itself
static com_operation_f opmode_default[] = {
  _op_hnd, _op_ack, _op_err, _op_cmd, _op_ctr, _op_dat
};
