// AVR Temperature Monitor -- Paolo Lucchesi
// AVR Command - set_resolution
#include <stddef.h> // NULL
#include "command.h"
#include "temperature_daemon.h"

#define COMMAND_NAME cmd_set_resolution

// Command starter
static uint8_t _start(const void *arg) {
  temperature_daemon_set_resolution(*((uint16_t*) arg));
  return CMD_RET_FINISHED;
}

static command_t _cmd = {
  .start   = _start,
  .iterate = NULL,
  .opmode  = NULL
};

command_t *COMMAND_NAME = &_cmd;
