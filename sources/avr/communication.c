// avrtmon
// Communication handler - Source File
// Paolo Lucchesi - Mon 17 Feb 2020 03:23:45 PM CET
// TODO: Find a way to correctly discard packets (e.g. flush the serial buffer
// and wait a while)
#include <avr/io.h>
#include <avr/interrupt.h>

#include "communication.h"
#include "command.h"
#include "packet.h"
#include "serial.h"
#include "led.h"

#define COMMAND_NONE COMMAND_COUNT


// Handle packet ids -- Represents the next id expected or to be used
// The variable name is long on purpose (global variable)
static uint8_t packet_global_id;

// Scaled timer variables for RTO
static volatile uint16_t rto_counter;
static volatile uint8_t  rto_elapsed; // 1 if it is elapsed, 0 if not
static volatile uint16_t rto_ongoing;

//static const uint16_t rto = 2 * RTO_DELIVERY_TIME + 4 * RTO_PROCESS_TIME (ms)
static const uint16_t rto = 500 / RTO_RESOLUTION; // Half second

// Communication Opmode variables
static com_operation_f opmode_default[];
static com_opmode_t opmode = opmode_default;

// Command currently in use
static command_id_t command_current = COMMAND_NONE;
static uint8_t command_notified = 0; // Handle command notifications


// Start the RTO timer, resetting it if it is ongoing, or stop it
static void rto_timer_start(void);
static void rto_timer_stop(void);

// End the current command
static inline void command_end(void) {
  command_current = COMMAND_NONE;
  communication_opmode_restore();
}


// Initialize communication module - Use Timer 3 for packet exchange timing
void communication_init(void) {
  serial_init(); // Initialize serial port

  // Initialize the RTO Timer
  // Set the prescaler to 1024
  TCCR3A = 0;
  TCCR3B = (1 << WGM52) | (1 << CS50) | (1 << CS52);

  // Set the OCR value
  OCR3A = (uint16_t)(15.625 * RTO_RESOLUTION);

  // Initialize variables
  packet_global_id = 0;
  rto_counter = 0;
  rto_elapsed = 0;
  rto_ongoing = 0;
}


// Single attempt to receive a packet
// Return 0 on a successful attempt, 1 otherwise
// TODO: Optimize to get all the available characters in a stroke
static uint8_t _recv_attempt(packet_t *p) {
  uint8_t *p_raw = (uint8_t*) p;

  // Start Round-Trip Time timeout
  if (!rto_ongoing) return 1;
  uint8_t type, id, size=0, received=0;

  while (1) {
    if (rto_elapsed) return E_TIMEOUT_ELAPSED;

    if (serial_rx_getchar(p_raw + received)) { // New data to process
      switch (++received) { // Received i-th byte

        case 1:
          // Early fail on mismatching ID
          type = packet_get_type(p);
          id = packet_get_id(p);
          if ((type == PACKET_TYPE_HND && id != 0) && id != packet_global_id)
            return E_ID_MISMATCH;
          break;

        case 2:
          // Check packet header integrity
          if (packet_check_header(p) != 0) return E_CORRUPTED_HEADER;
          size = packet_get_size(p);
          break;

        default:
          if (received >= size) // Last byte
            return (packet_check_crc(p) != 0) ? E_CORRUPTED_CHECKSUM : E_SUCCESS;
          break;
      }
    }
  }
}


// Get an incoming packet if data is available on the serial port (blocking)
// Returns 0 on success, 1 on failure
// TODO: Handle lost ACK (i.e. host retransmits packet)
uint8_t communication_recv(packet_t *p) {
  static packet_t response[1];

  for (uint8_t attempt=0; attempt < MAXIMUM_RECV_ATTEMPTS; ++attempt) {
    rto_timer_start();
    uint8_t ret = _recv_attempt(p);

    switch (ret) {

      case E_SUCCESS:
        packet_ack(p, response);
        serial_tx(response, PACKET_MIN_SIZE);
        rto_timer_stop();
        if (packet_get_type(p) == PACKET_TYPE_HND)
          packet_global_id = 1;
        else packet_global_id = packet_next_id(packet_global_id);
        return 0;

      case E_CORRUPTED_HEADER:
      case E_CORRUPTED_CHECKSUM:
        packet_err(p, response);
        serial_tx(response, PACKET_MIN_SIZE);
        //serial_rx_reset(); // Discard remaining data to process (TODO)
        rto_timer_stop();
        break;

      default: break; // Discard on RTO timeout or mismatching ID
    }
  }

  return 1; // Too many consecutive failures
}


// Send a packet (blocking)
// Returns 0 if the packet is sent correctly, 1 otherwise
uint8_t communication_send(const packet_t *p) {
  const uint8_t size = packet_get_size(p);
  if (!p || !size || size > sizeof(packet_t))
    return 1;

  for (uint8_t attempt=0; attempt < MAXIMUM_SEND_ATTEMPTS; ++attempt) {
    rto_timer_start(); // Restart RTO timer for each attempt

    // Blocking send
    serial_tx(p, size);
    while (serial_tx_ongoing()) ;

    // Attempt to receive ACK/ERR
    // Assertion on ACK/ERR id is made inside the receive attempt function
    // If any packet different from an ACK one is received, take it as a failure
    // TODO: Assert correct ID
    static packet_t response[1];
    uint8_t ret = _recv_attempt(response);
    if (rto_ongoing) rto_timer_stop();
    if (ret == E_SUCCESS && packet_get_type(response) == PACKET_TYPE_ACK) {
      packet_global_id = packet_next_id(packet_global_id);
      command_notified = 1;
      return 0;
    }
  }

  return 1; // Too many consecutive failures
}


// Send an in-place crafted packet
// Returns 0 if the packet is sent correctly, 1 otherwise
uint8_t communication_craft_and_send(packet_type_t type, const uint8_t *data,
    uint8_t data_size) {
  static packet_t p[1];
  if (packet_craft(packet_global_id, type, data, data_size, p) != 0)
    return 1;
  return communication_send(p);
}


// Transfer control flow to the communication layer
void communication_handler(void) {
  if (command_notified) {
    command_notified = 0;
    if (command_iterate(command_current, NULL) != CMD_RET_ONGOING)
      command_end();
  }

  if (!serial_rx_available()) return;

  // If data is available, attempt to receive a new packet
  static packet_t p[1];
  if (communication_recv(p) != 0) return;

  // The incoming packet have been received correctly
  const uint8_t type = packet_get_type(p);
  com_operation_f action = opmode[type] ? opmode[type] : opmode_default[type];
  if (action && action(p) != CMD_RET_ONGOING)
    communication_opmode_restore();
}


// Switch communication opmode
void communication_opmode_switch(const com_opmode_t opmode_new) {
  if (opmode_new) opmode = opmode_new;
}

// Restore the communication opmode to the default one
void communication_opmode_restore(void) {
  opmode = opmode_default;
}



// RTO Timer ISR -- Timer 3 is used
ISR(TIMER3_COMPA_vect) {
  if (++rto_counter >= rto) {
    TIMSK3 &= ~(1 << OCIE3A);
    rto_elapsed = 1;
    rto_ongoing = 0;
  }
}

// Start the timer, resetting it if it is ongoing
static void rto_timer_start(void) {
  if (rto_ongoing) rto_timer_stop();
  rto_counter = 0;
  rto_ongoing = 1;
  rto_elapsed = 0;
  TIMSK3 |= (1 << OCIE3A);
}

// Stop the timer
static void rto_timer_stop(void) {
  TIMSK3 &= ~(1 << OCIE3A);
  rto_ongoing = 0;
}



// Default, built-in communication opmode
// Operation for PACKET_TYPE_HND -- Reset the communication environment
// TODO: Restore what is necessary
static uint8_t _op_hnd(const packet_t *rx_pack) {
  communication_opmode_restore();
  //serial_rx_reset();
  //serial_tx_reset();
  return CMD_RET_ONGOING;
}

// Operation for PACKET_TYPE_CMD -- Change operation table and execute command
static uint8_t _op_cmd(const packet_t *rx_pack) {
  const command_payload_t *payload = (const command_payload_t*) rx_pack->data;
  command_id_t cmd = payload->id;
  uint8_t ret = command_start(cmd, (const void*) payload->arg);
  if (ret == CMD_RET_ONGOING)
    command_current = cmd;
  return ret;
}

// The opmode itself
static com_operation_f opmode_default[] = {
  _op_hnd, NULL, NULL, _op_cmd, NULL, NULL
};
