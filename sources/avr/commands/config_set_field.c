// AVR Temperature Monitor -- Paolo Lucchesi
// AVR Command - config_set_field
#include <stddef.h>  // NULL
#include <avr/io.h>

#include "command.h"
#include "config.h"

#define COMMAND_NAME cmd_config_set_field


// Command starter
static uint8_t _start(const void *arg) {
  const config_setter_t *_arg = arg;
  config_set(_arg->id, _arg->value);
  config_save_field(_arg->id);
  return CMD_RET_FINISHED;
}

static command_t _cmd = {
  .start   = _start,
  .iterate = NULL,
  .opmode  = NULL
};

command_t *COMMAND_NAME = &_cmd;
