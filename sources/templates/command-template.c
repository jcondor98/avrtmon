// avrtmon
// Command interface - Single command template
// Paolo Lucchesi - Wed 23 Oct 2019 04:56:11 PM CEST
#include "command.h"
#include "communication.h"

// Command name -- You ought choose something in the form 'cmd_something'
#define COMMAND_NAME cmd_foo


// Command starter
void _start(const void *arg) {
  // Put the start code here
}


/* Decomment this if you need to define an opmode for the command, else you can
 * delete this snippet
// Single command iteration
static uint8_t _iterate(const void *arg) {
  // Put the iteration code here
}
*/


/* Decomment this if you need to define an opmode for the command, else you can
 * delete this snippet
// Operation for PACKET_TYPE_CMD
static uint8_t _op_cmd(const packet_t *p) {

}

// Operation for PACKET_TYPE_CTR
static uint8_t _op_ctr(const packet_t *p) {

}

// Operation for PACKET_TYPE_DAT
static uint8_t _op_dat(const packet_t *p) {

}

// The opmode itself
// Replace unrequired functions with NULL
// WARNING: With NULL, the communication layer will fallback to a default
// operation (which is good), but with an existent yet dummy function it won't!
static const com_operation_f _opmode[] = {
  NULL, NULL, NULL, _op_cmd, _op_ctr, _op_dat
};
*/

static command_t COMMAND_NAME = {
  .start   = /* _start   or NULL */;
  .iterate = /* _iterate or NULL */;
  .opmode  = /* _opmode  or NULL */;
};
