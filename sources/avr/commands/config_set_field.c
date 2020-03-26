// avrtmon
// Command interface - Single command template
// Paolo Lucchesi - Wed 23 Oct 2019 04:56:11 PM CEST
#include "command.h"
#include "config.h"

#define COMMAND_NAME cmd_config_set_field


// Command starter
// TODO: Assertion on field size?
static uint8_t _start(const void *arg) {
  const config_setter_t *_arg = arg;
  config_set(_arg->id, _arg->value);
  return CMD_RET_FINISHED;
}


static command_t _cmd = {
  .start   = _start,
  .iterate = NULL,
  .opmode  = NULL
};

command_t *COMMAND_NAME = &_cmd;
