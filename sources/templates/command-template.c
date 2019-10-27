// avrtmon
// Command interface - Single command template
// Paolo Lucchesi - Wed 23 Oct 2019 04:56:11 PM CEST
#include "command.h"
#include "communication.h"

// Command name -- You ought choose something in the form 'cmd_something'
#define COMMAND_NAME cmd_foo


// Command starter
void _start(const void *arg) {
  // Put the start command code here
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

static command_t COMMAND_NAME = {
  .start = _start,
  .opmode = /* _opmode or NULL */;
};
