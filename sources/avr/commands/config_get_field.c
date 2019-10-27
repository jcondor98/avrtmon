// avrtmon
// Command interface - Single command template
// Paolo Lucchesi - Wed 23 Oct 2019 04:56:11 PM CEST
#include <string.h>
#include "command.h"
#include "config.h"

#define COMMAND_NAME cmd_config_get_field


// Command starter
void _start(const void *arg) {
  config_field_t field = *((config_field_t*) arg);
  uint8_t field_size = config_get_size(field);

  uint8_t val[field_size];  // Field value will be stored here
  if (config_get(field, val) != 0)  // Send 0 if the field does not exist
    memset(val, 0, PACKET_DATA_MAX_SIZE);

  com_craft_and_send(PACKET_TYPE_DAT, val, field_size);
}

static command_t _cmd = {
  .start = _start,
  .opmode = NULL
};

command_t *COMMAND_NAME = &_cmd;
