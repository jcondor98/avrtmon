// AVR Temperature Monitor -- Paolo Lucchesi
// Command interface - Single command template
#include <stddef.h>  // NULL
#include <avr/io.h>

#include "command.h"
#include "config.h"

#define COMMAND_NAME cmd_config_set_field

// Command starter
static uint8_t _start(const void *arg) {
  uint8_t ret = 0;
  const config_setter_t *_arg = arg;
  config_set(_arg->id, _arg->value);
  ret |= config_save_field(_arg->id);
  if (ret) PORTB |= (1 << 7);
  return CMD_RET_FINISHED;
}

static command_t _cmd = {
  .start   = _start,
  .iterate = NULL,
  .opmode  = NULL
};

command_t *COMMAND_NAME = &_cmd;
