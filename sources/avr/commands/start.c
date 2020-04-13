// AVR Temperature Monitor -- Paolo Lucchesi
// AVR Command - start
#include <stddef.h> // NULL
#include "command.h"
#include "temperature_daemon.h"

#define COMMAND_NAME cmd_start

// Command starter
static uint8_t _start(const void *arg) {
  if (!temperature_daemon_ongoing())
    temperature_daemon_start(1);
  return CMD_RET_FINISHED;
}

static command_t _cmd = {
  .start   = _start,
  .iterate = NULL,
  .opmode  = NULL
};

command_t *COMMAND_NAME = &_cmd;
