// AVR Temperature Monitor -- Paolo Lucchesi
// Command interface - Single command template
#include <string.h>
#include "communication.h"
#include "command.h"
#include "config.h"

#define COMMAND_NAME cmd_config_get_field


// Command starter
static uint8_t _start(const void *arg) {
  config_field_t field = *((config_field_t*) arg);
  uint8_t field_size = config_get_size(field);
  uint8_t val[field_size];  // Field value will be stored here

  // Send the field value, or an empty CTR packet if the field does not exist
  if (config_get(field, val) != 0)  
    communication_craft_and_send(PACKET_TYPE_CTR, NULL, 0);
  else
    communication_craft_and_send(PACKET_TYPE_DAT, val, field_size);

  return CMD_RET_FINISHED;
}

static command_t _cmd = {
  .start   = _start,
  .iterate = NULL,
  .opmode  = NULL
};

command_t *COMMAND_NAME = &_cmd;
