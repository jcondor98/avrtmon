// AVR Temperature Monitor -- Paolo Lucchesi
// Command interface - Single command template
#include <stddef.h>  // NULL
#include "command.h"
#include "temperature.h"
#include "temperature_daemon.h"

#define COMMAND_NAME cmd_temperatures_reset

// Command starter
static uint8_t _start(const void *arg) {
  temperature_daemon_stop(1);
  temperature_db_reset();
  return CMD_RET_FINISHED;
}

static command_t _cmd = {
  .start   = _start,
  .iterate = NULL,
  .opmode  = NULL
};

command_t *COMMAND_NAME = &_cmd;
