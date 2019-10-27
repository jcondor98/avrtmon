// avrtmon
// Command interface - Head file
// Paolo Lucchesi - Sun 06 Oct 2019 05:13:56 PM CEST
#include <string.h>
#include "command.h"
#include "communication.h"


// Commands table -- Each command is identified by a pointer to its data struct
static command_t *cmd_table[COMMAND_COUNT];

// Imported commands
extern command_t *cmd_config_get_param;
extern command_t *cmd_config_set_param;
extern command_t *cmd_temperatures_download;
extern command_t *cmd_temperatures_reset;


// Execute a command, given its ID and an optional argument
uint8_t command_exec(command_id_t id, const void *arg) {
  if (id >= COMMAND_COUNT)
    return 1;

  if (cmd_table[id]->opmode)
    com_opmode_switch(cmd_table[id]->opmode);

  cmd_table[id]->start(arg);
  return 0;
}


// Initialize the command table
void command_init(void) {
  cmd_table[CMD_CONFIG_GET_FIELD]      = cmd_config_get_param;
  cmd_table[CMD_CONFIG_SET_FIELD]      = cmd_config_set_param;
  cmd_table[CMD_TEMPERATURES_DOWNLOAD] = cmd_temperatures_download;
  cmd_table[CMD_TEMPERATURES_RESET]    = cmd_temperatures_reset;
}
