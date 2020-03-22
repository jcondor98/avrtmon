// avrtmon
// Communication layer (host-side) - Source file
// Paolo Lucchesi - Tue 25 Feb 2020 12:10:03 PM CET
// TODO: Correctly handle IDs
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "host/communication.h"
#include "host/serial.h"
#include "host/debug.h"
#include "packet.h"


#define ONE_MSEC 1000000 // One millisecond in nanoseconds

// Timer entity, i.e. a data structure containing everything related to a timer
typedef struct _timer_entity_s {
  timer_t id;
  struct itimerspec timeval;
  struct sigevent   sig_ev;
  struct sigaction  sig_act;
  volatile unsigned char elapsed;
  volatile unsigned char ongoing;
} timer_entity_t;


// The RTO timer itself
static timer_entity_t rto_timer;

// Keep track of the current expected packet ID
static unsigned char packet_global_id;


// Timer functions -- Source at the bottom of this source file
static inline int  rto_timer_init(void);
static inline void rto_timer_destroy(void);
static void rto_timer_start(void);
static void rto_timer_stop(void);

// Attempt to receive a packet
static unsigned char _recv_attempt(serial_context_t *ctx, packet_t *p);


// Initialize the communication module -- In practice, init the RTO timer
int communication_init(void) {
  return rto_timer_init();
}

// Cleanup for an initialized communication module
// TODO: What if the module was not initialized?
void communication_cleanup(void) {
  rto_timer_destroy();
}

// Estabilish a connection, sending a handshake (HND) packet
// Return 0 on success, 1 on failure
int communication_connect(serial_context_t *ctx) {
  packet_global_id = 0;
  return (communication_craft_and_send(ctx, PACKET_TYPE_HND, NULL, 0) != 0) ? 1 : 0;
}


// Send a packet
// Returns 0 on success, 1 on failure
// Never use for HND, ACK or ERR packet types
int communication_send(serial_context_t *ctx, const packet_t *p) {
  uint8_t size = packet_get_size(p);
  if (!ctx || !p || !size || size > sizeof(packet_t))
    return 1;

  debug {
    puts("\nSending packet");
    packet_print(p);
  }

  // Flush the serial RX buffers before beginning the communication
  // TODO: Is this effective?
  serial_rx_flush(ctx);

  for (uint8_t attempt=0; attempt < MAXIMUM_SEND_ATTEMPTS; ++attempt) {
    rto_timer_start();

    // Blindly send the packet on the serial port
    serial_tx(ctx, p, size);
    debug err_log("Blindly wrote data");

    // Attempt to receive ACK/ERR
    // Assertion on ACK/ERR id is made inside the receive attempt function
    // If any packet different from an ACK one is received, take it as a failure
    packet_t response[1];
    uint8_t ret = _recv_attempt(ctx, response);
    debug err_log("_recv_attempt() returned %hhd", ret);

    debug {
      if (ret == E_TIMEOUT_ELAPSED) err_log("Timeout elapsed");
      else if (ret != E_SUCCESS) err_log("Bad response received");
      else {
        printf("Valid response received");
        packet_print(response);
      }
    }

    if (ret == E_SUCCESS && packet_get_type(response) == PACKET_TYPE_ACK) {
      rto_timer_stop();
      packet_global_id = packet_next_id(packet_global_id);
      debug err_log("Packet succesfully sent");
      return 0;
    }
  }

  debug err_log("Too many consecutive failures");
  return 1; // Too many consecutive failures
}


// Receive a packet
// Returns 0 on success, 1 on failure
// Never use for HND, ACK or ERR packet types
int communication_recv(serial_context_t *ctx, packet_t *p) {
  if (!ctx || !p) return 1;
  packet_t response[1];

  for (unsigned char attempt=0; attempt < MAXIMUM_RECV_ATTEMPTS; ++attempt) {
    rto_timer_start();
    unsigned char ret = _recv_attempt(ctx, p);
    debug err_log("_recv_attempt() returned %hhd", ret);

    switch (ret) {

      case E_SUCCESS:
        packet_ack(p, response);
        serial_tx(ctx, response, PACKET_MIN_SIZE);
        rto_timer_stop();
        packet_global_id = packet_next_id(packet_global_id);
        debug {
          err_log("Packet received successfully");
          packet_print(p);
        }
        return 0;

      case E_CORRUPTED_HEADER:
      case E_CORRUPTED_CHECKSUM:
        packet_err(p, response);
        serial_tx(ctx, response, PACKET_MIN_SIZE);
        rto_timer_stop();
        debug err_log("Attempt %d failed: corrupted packet", attempt + 1);
        break;

      // Discard on RTO timeout or mismatching ID
      default:
        debug err_log("Attempt %d failed: RTO timeout or id mismatch", attempt + 1);
        break;
    }
  }

  return 1; // Too many consecutive failures
}


// Craft a packet in-place and send it
// Returns 0 on success, 1 on failure
// Never use for HND, ACK or ERR packet types
int communication_craft_and_send(serial_context_t *ctx, unsigned char type,
    const unsigned char *data, unsigned char data_size) {
  packet_t p[1];
  if (packet_craft(packet_global_id, type, data, data_size, p) != 0)
    return 1;
  return communication_send(ctx, p);
}


// Craft a command packet in-place and send it
// Return 0 if the packet was sent correctly, 1 otherwise
int communication_cmd(serial_context_t *ctx, command_id_t cmd,
    const void *arg, unsigned arg_size) {
  if (!ctx || cmd >= COMMAND_COUNT || (arg && !arg_size) || (!arg && arg_size)
      || arg_size > PACKET_DATA_MAX_SIZE - sizeof(command_id_t))
    return 1;

  unsigned char _payload[PACKET_DATA_MAX_SIZE];
  command_payload_t *payload = (command_payload_t*) _payload;

  payload->id = cmd;
  if (arg) memcpy(payload->arg, arg, arg_size);

  return communication_craft_and_send(ctx, PACKET_TYPE_CMD, _payload,
      sizeof(command_id_t) + arg_size);
}


// Attempt to receive a packet
// Return an appropriate error code (E_SUCCESS on success)
static unsigned char _recv_attempt(serial_context_t *ctx, packet_t *p) {
  unsigned char *p_raw = (unsigned char*) p;
  unsigned char id, size=0, received=0;

  while (1) {
    if (rto_timer.elapsed) return E_TIMEOUT_ELAPSED;

    if (!serial_rx_getchar(ctx, p_raw + received))
      continue;

    switch (++received) { // Received i-th byte

      case 1:
        // Early fail on mismatching ID
        id = packet_get_id(p);
        if (id != packet_global_id) return E_ID_MISMATCH;
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



// Timer interface implementation
// This part of the communication module uses a POSIX per-process timer, thus
// it is not reentrant
// TODO: Check errors

// RTO Signal handler
static void rto_sig_handler(int sig) {
  rto_timer.elapsed = 1;
  rto_timer.ongoing = 0;
  //debug err_log("RTO Timer elapsed");
}


// Initialize the RTO timer and set a sigaction handler for SIGUSR1
// Returns 0 on success, 1 on failure
// TODO: Correct error checking
static inline int rto_timer_init(void) {
  // Initialize data structure
  rto_timer = (timer_entity_t) {
    .timeval = {
      { 0, 0 }, // One-shot timer
      //{ 0, RTO_RESOLUTION * ONE_MSEC * (2*RTO_DELIVERY_TIME + 4*RTO_PROCESS_TIME) }
      { 0, 500 * ONE_MSEC } // TODO: Lower?
    },
  .sig_ev  = { SIGEV_SIGNAL, SIGUSR1 },
  .sig_act = (struct sigaction) { .sa_handler = rto_sig_handler }
  };

  // Create timer
  err_check(timer_create(CLOCK_REALTIME, &rto_timer.sig_ev, &rto_timer.id) != 0,
      1, "Unable to create timer");

  // Handle SIGUSR1 -- TODO: Destroy timer on failure
  err_check(sigaction(SIGUSR1, &rto_timer.sig_act, NULL) != 0,
      1, "Unable to set signal handler (sigaction) for SIGUSR1");

  return 0;
}

static inline void rto_timer_destroy(void) {
  // TODO: Reset signal handler for SIGUSR1
  // TODO: Handle ongoing timer
  timer_delete(rto_timer.id);
}

// Start (i.e. arm) the timer
static void rto_timer_start(void) {
  if (rto_timer.ongoing)
    rto_timer_stop();
  rto_timer.elapsed = 0;
  timer_settime(rto_timer.id, 0, &rto_timer.timeval, NULL);
  //debug err_log("RTO Timer started");
}

// Stop (i.e. disarm) the timer
static void rto_timer_stop(void) {
  static const struct itimerspec disarmer = { 0 };
  if (!rto_timer.ongoing) return;
  timer_settime(rto_timer.id, 0, &disarmer, NULL);
  rto_timer.ongoing = 0;
  //debug err_log("RTO Timer stopped");
}
