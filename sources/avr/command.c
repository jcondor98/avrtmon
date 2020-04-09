// AVR Temperature Monitor -- Paolo Lucchesi
// Command interface - Head file
#include <string.h>
#include "command.h"
#include "communication.h"


// Commands table -- Each command is identified by a pointer to its data struct
static command_t *cmd_table[COMMAND_COUNT];

// Imported commands
extern command_t *cmd_config_get_field;
extern command_t *cmd_config_set_field;
extern command_t *cmd_temperatures_download;
extern command_t *cmd_temperatures_reset;
extern command_t *cmd_echo;


// Execute the start routine of a command, given its ID and an optional argument
// Returns a 'command_retval_t' code
uint8_t command_start(command_id_t id, const void *arg) {
  if (id >= COMMAND_COUNT) return CMD_RET_NOT_EXISTS;
  uint8_t has_opmode  = cmd_table[id]->opmode  != NULL ? 1 : 0;
  uint8_t has_iterate = cmd_table[id]->iterate != NULL ? 1 : 0;

  if (has_opmode) communication_opmode_switch(cmd_table[id]->opmode);

  // A command with no 'start', 'iterate' and 'opmode' should not be created,
  // nevertheless, better safe than sorry...
  if (!cmd_table[id]->start)
    return (has_opmode || has_iterate) ? CMD_RET_ONGOING : CMD_RET_FINISHED;

  return cmd_table[id]->start(arg);
}


// Execute the routine of a command, given its ID and an optional argument
uint8_t command_iterate(command_id_t id, const void *arg) {
// Returns a 'command_retval_t' code
  if (id >= COMMAND_COUNT)     return CMD_RET_NOT_EXISTS;
  if (!cmd_table[id]->iterate) return CMD_RET_NOT_IMPLEMENTED;
  return cmd_table[id]->iterate(arg);
}


// Initialize the command table
void command_init(void) {
  cmd_table[CMD_CONFIG_GET_FIELD]      = cmd_config_get_field;
  cmd_table[CMD_CONFIG_SET_FIELD]      = cmd_config_set_field;
  cmd_table[CMD_TEMPERATURES_DOWNLOAD] = cmd_temperatures_download;
  cmd_table[CMD_TEMPERATURES_RESET]    = cmd_temperatures_reset;
  cmd_table[CMD_ECHO]                  = cmd_echo;
}
