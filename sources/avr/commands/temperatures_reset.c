// avrtmon
// Command interface - Single command template
// Paolo Lucchesi - Wed 23 Oct 2019 04:56:11 PM CEST
#include "command.h"
#include "temperature.h"

#define COMMAND_NAME cmd_temperatures_reset


// Command starter
void _start(const void *arg) {
  temperature_db_reset();
}

static command_t _cmd = {
  .start = _start,
  .opmode = NULL
};

command_t *COMMAND_NAME = &_cmd;
