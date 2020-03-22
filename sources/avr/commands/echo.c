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
  const uint8_t *msg = (const uint8_t*) arg;
  uint8_t len = 0;
  while (*(msg + len) && len < ARG_MAX_LEN)
    ++len;
  communication_craft_and_send(PACKET_TYPE_DAT, msg, len);
}


static command_t _cmd = {
  .start = _start,
  .opmode = NULL
};

command_t *COMMAND_NAME = &_cmd;
