// avrtmon
// Command interface - Command: echo
// Paolo Lucchesi - Wed 27 Nov 2019 09:33:07 PM CET
#include "command.h"
#include "communication.h"

// Command name
#define COMMAND_NAME cmd_echo
#define ARG_MAX_LEN (PACKET_DATA_MAX_SIZE - 2)


// Command starter
void _start(const void *arg) {
  uint8_t len = 0, *_arg = arg;
  while (*(_arg + len) && len < ARG_MAX_LEN)
    ++len;
  com_craft_and_send(PACKET_TYPE_DAT, arg, len);
}

/* Decomment this if you need to define an opmode for the command, else you can
 * delete this snippet
// Operation for PACKET_TYPE_ACK
static void _op_ack(void) {

}

// Operation for PACKET_TYPE_CMD
static void _op_cmd(void) {

}

// Operation for PACKET_TYPE_CTR
static void _op_ctr(void) {

}

// Operation for PACKET_TYPE_DAT
static void _op_dat(void) {

}

// The opmode itself
// Replace unrequired functions with NULL
// WARNING: With NULL, the communication layer will fallback to a default
// operation (which is good), but with an existent yet dummy function it won't!
static const com_operation_f _opmode[] = {
  NULL, _op_ack, NULL, _op_cmd, _op_ctr, _op_dat
};
*/

static command_t _cmd = {
  .start = _start,
  .opmode = NULL
};

command_t *COMMAND_NAME = &_cmd;
