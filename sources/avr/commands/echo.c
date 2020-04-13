// AVR Temperature Monitor -- Paolo Lucchesi
// AVR Command - echo
#include <stddef.h>  // NULL
#include "command.h"
#include "communication.h"

#define COMMAND_NAME cmd_echo
#define ARG_MAX_LEN (PACKET_DATA_MAX_SIZE - 2)

// Command starter
static uint8_t _start(const void *arg) {
  const uint8_t *msg = (const uint8_t*) arg;
  uint8_t len = 0;
  while (*(msg + len) && len < ARG_MAX_LEN)
    ++len;
  communication_craft_and_send(PACKET_TYPE_DAT, msg, len);
  return CMD_RET_FINISHED;
}

static command_t _cmd = {
  .start   = _start,
  .iterate = NULL,
  .opmode  = NULL
};

command_t *COMMAND_NAME = &_cmd;
