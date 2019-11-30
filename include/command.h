// avrtmon
// Command interface - Head file
// Paolo Lucchesi - Sun 06 Oct 2019 05:14:06 PM CEST
#ifndef __COMMAND_INTERFACE_H
#define __COMMAND_INTERFACE_H
#include "config.h"

#ifdef AVR
#include "communication.h"
#endif


// Command id data type definition
// i.e. All the different executable commands
#define COMMAND_COUNT 5
typedef enum COMMAND_ID_E {
  CMD_CONFIG_GET_FIELD,
  CMD_CONFIG_SET_FIELD,
  CMD_TEMPERATURES_DOWNLOAD,
  CMD_TEMPERATURES_RESET,
  CMD_ECHO
} command_id_t;

// Command payload transported via CMD packet from host to AVR
typedef struct _command_payload_s {
  uint8_t id;     // Require a fixed size
  uint8_t arg[];  // This can be casted to any required type for a specific cmd
} command_payload_t;

// Command payload argument: set a configuration field to an arbitrary value
typedef struct _config_setter_s {
  uint8_t id;
  uint8_t value[];
} config_setter_t;

typedef uint16_t command_download_arg_t;

#ifdef AVR

// Command function to be executed as its "launcher"
typedef void (*command_f)(const void *arg);

// Command data type definition
typedef struct _command_s {
  command_f start;
  com_opmode_t opmode;
} command_t;

// Initialize the commands table
void command_init(void);

// Execute a command, given its ID and an optional argument
uint8_t command_exec(command_id_t id, const void *arg);

#endif    // AVR specific types and functions
#endif    // __COMMAND_INTERFACE_H
