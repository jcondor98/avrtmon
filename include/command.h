// AVR Temperature Monitor -- Paolo Lucchesi
// Command interface - Head file
#ifndef __COMMAND_MODULE_H
#define __COMMAND_MODULE_H
#include "config.h"

// Command id data type definition
// i.e. All the different executable commands
#define COMMAND_COUNT 9
typedef enum COMMAND_ID_E {
  CMD_CONFIG_GET_FIELD,
  CMD_CONFIG_SET_FIELD,
  CMD_TEMPERATURES_DOWNLOAD,
  CMD_TEMPERATURES_RESET,
  CMD_SET_RESOLUTION,
  CMD_SET_INTERVAL,
  CMD_START,
  CMD_STOP,
  CMD_ECHO
} command_id_t;

// Return value of a command action function
// A finished command MUST return CMD_RET_FINISHED
typedef enum COMMAND_RETVAL_E {
  CMD_RET_FINISHED,         // Finished, communication module must cleanup
  CMD_RET_ONGOING,
  CMD_RET_ERROR,
  CMD_RET_NOT_EXISTS,       // Command ID is not valid
  CMD_RET_NOT_IMPLEMENTED   // Not an error, 'iterate' is not implemented
} cmd_retval_t;

// Command payload transported via CMD packet from host to AVR
typedef struct _command_payload_s {
  uint8_t id;     // Command ID
  uint8_t arg[];  // This can be casted to any required type for a specific cmd
} command_payload_t;

// Command payload argument: set a configuration field to an arbitrary value
typedef struct _config_setter_s {
  uint8_t id;       // Configuration field ID
  uint8_t value[];  // Configuration field value (variable size)
} config_setter_t;

typedef uint16_t command_download_arg_t;


#ifdef AVR // AVR specific stuff
#include "communication.h"

// Command function to be executed as its "launcher"
typedef uint8_t (*command_action_f)(const void *arg);

// Command data type definition
typedef struct _command_s {
  command_action_f start;   // Executed the first time a command is launched
  command_action_f iterate; // Executed every time the command "receives an event"
  com_opmode_t opmode;
} command_t;

// Initialize the commands table
void command_init(void);

// Start a command, given its ID and an optional argument
// This will eventually alter the current opmode
uint8_t command_start(command_id_t id, const void *arg);

// Execute a single iteration of a command
uint8_t command_iterate(command_id_t id, const void *arg);

#endif  // AVR specific types and functions

#endif  // __COMMAND_MODULE_H
