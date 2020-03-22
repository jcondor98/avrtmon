// avrtmon
// Program shell (i.e. command line) - Commands
// Paolo Lucchesi - Tue 05 Nov 2019 01:36:25 AM CET
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "host/shell.h"
#include "host/list.h"
#include "host/serial.h"
#include "host/temperature.h"
#include "host/communication.h"


// Declare a shell_storage_t variable casted from an opaque pointer
#define _storage_cast(st,opaque) shell_storage_t *st=(shell_storage_t*)(opaque)

#define DEBUG_RECV_MSG_SIZE 2048

// Type definition for the internal shell storage
typedef struct _shell_storage_s {
  serial_context_t *serial_ctx;
  list_t *dbs;
} shell_storage_t;


// Allocate and initialize a shell storage
// Returns an opaque pointer to the allocated storage, or NULL on failure
void *shell_storage_new(void) {
  shell_storage_t *st = malloc(sizeof(shell_storage_t));
  if (!st) return NULL;

  st->dbs = list_new();
  if (!st->dbs) {
    free(st);
    return NULL;
  }

  st->serial_ctx = NULL;  // i.e. not connected
  return (void*) st;
}

// Cleanup this shell environment (i.e. free memory, close descriptors...)
// Also frees the shell storage data structure itself
void shell_cleanup(shell_t *s) {
  if (!s) return;
  _storage_cast(st, s->storage);

  // Close the connection with the tmon, if any
  if (st->serial_ctx && serial_close(st->serial_ctx) != 0)
    fputs("Error: Could not close the tmon file descriptor\n", stderr);

  // Free the storage
  list_delete(st->dbs);
  free(st);
}



// CMD: echo
// Usage: echo [arg1 arg2 ...]
// Print back the arguments given
int echo(int argc, char *argv[], void *storage) {
  for (int i=1; i < argc; ++i) {
    fputs(argv[i], stdout);
    putchar(i == argc-1 ? '\n' : ' ');
  }
  return 0;
}


// CMD: connect
// Usage: connect <device_path>
// Connect to an avrtmon, given its device file (usually under /dev)
// If it is already connected, reconnect
int connect(int argc, char *argv[], void *storage) {
  _storage_cast(st, storage);
  if (argc < 2) return 1;

  if (st->serial_ctx)
    fputs("Open serial context found; reconnecting\n", stderr);
  else { // Open a descriptor for the tmon
    st->serial_ctx = serial_open(argv[1]);
    sh_error_on(!st->serial_ctx, "Unable to connect to the tmon", 2);
  }

  // Estabilish the connection
  sh_error_on(communication_connect(st->serial_ctx) != 0,
      "Could not send a handshake packet", 3);

  return 0;
}


// CMD: disconnect
// Usage: disconnect
// Close an existing connection - Has no effect on the tmon
int disconnect(int argc, char *argv[], void *storage) {
  _storage_cast(st, storage);
  if (argc != 1) return 1;

  sh_error_on(!st->serial_ctx, "tmon is not connected", 2);

  int ret = serial_close(st->serial_ctx);
  st->serial_ctx = NULL; // i.e. set to disconnected
  sh_error_on(ret != 0, "Could not close the tmon file descriptor", 3);

  return 0;
}

/* TODO: Correct
// CMD: download
// Usage: download
// Download all the temperatures from the tmon. creating a new database
int download(int argc, char *argv[], void *storage) {
  _storage_cast(st, storage);
  if (argc > 1) return 1;
  if (st->tmon_fd < 0) {  // i.e. not connected
    fputs("Error: tmon is not connected\n", stderr);
    return 2;
  }

  // Max number of temperatures contained in a DB
  static const unsigned temp_burst = PACKET_DATA_MAX_SIZE / sizeof(uint16_t);

  // Store the ID for the new database
  static size_t db_id = 0;

  // Send a download command to the tmon
  serial_handle(serial_cmd(CMD_TEMPERATURES_DOWNLOAD, NULL, 0, st->tmon_fd),
      "Could not send CMD packet");

  // Get the number of temperature from the tmon response (as temp_count)
  packet_t pack_rx;
  serial_handle(serial_recv(st->tmon_fd, &pack_rx),
      "Could not receive the number of temperatures to fetch");

  command_payload_t *payload_in = (command_payload_t*) pack_rx.data;
  command_download_arg_t temp_count =
    *((command_download_arg_t*)(payload_in->arg));

  if (temp_count) {
    // Create a new database for the incoming temperatures
    temperature_db_t *db = temperature_db_new(db_id, temp_count, NULL);

    // Get and add the temperatures
    unsigned i = 0;
    while (i < temp_count) {
      serial_handle(serial_recv(st->tmon_fd, &pack_rx), "Could not receive data");
      do {
        temperature_register(db, i,
            temperature_raw2float(pack_rx.data[i % temp_burst]));
        ++i;
      } while (i % temp_burst && i < temp_count);
    }

    // Add the database to the global storage
    list_add(st->dbs, db);
  }

  // Get the last CTR packet 
  serial_handle(serial_recv(st->tmon_fd, &pack_rx),
      "Could not receive last CTR packet");

  return 0;
}
*/


// CMD: tmon-reset
// Usage: tmon-reset
// Reset the internal temperatures DB of the tmon
int tmon_reset(int argc, char *argv[], void *storage) {
  _storage_cast(st, storage);
  if (argc != 1) return 1;

  sh_error_on(!st->serial_ctx, "tmon is not connected", 2);
  sh_error_on(
      communication_cmd(st->serial_ctx, CMD_TEMPERATURES_RESET, NULL, 0) != 0,
      "Could not send CMD packet", 2);

  return 0;
}


// CMD: tmon-config
// Usage: tmon-config list
//        tmon-config get <field>
//        tmon-config set <field> <value>
// Manipulate the tmon configuration
// TODO: Test
int tmon_config(int argc, char *argv[], void *storage) {
  _storage_cast(st, storage);
  if (argc < 2 || argc > 4) return 1;

  // List the configuration fields
  if (strcmp(argv[1], "list") == 0) {
    if (argc != 2) return 1;
    for (config_field_t f=0; f < CONFIG_FIELD_COUNT; ++f)
      puts(config_field_str(f));
    return 0;
  }

  // Get a config field
  else if (strcmp(argv[1], "get") == 0) {
    if (argc != 3) return 1;
    config_field_t f;
    sh_error_on(config_field_id(argv[2], &f) != 0, "Invalid config field", 2);

    // Send a CONFIG_GET_FIELD command to the tmon
    sh_error_on(
        communication_cmd(st->serial_ctx, CMD_CONFIG_GET_FIELD, &f, sizeof(config_field_t)),
        "Could not send CMD packet", 3);

    // Retrieve the field from the tmon
    packet_t pack_rx[1];
    sh_error_on(communication_recv(st->serial_ctx, pack_rx),
        "Could not receive config field value", 3);

    // Handle the received config field value
    // We assume that every config field is an integer
    void *field_val = (void*) pack_rx->data;
    size_t field_size = packet_get_size(pack_rx) - PACKET_HEADER_SIZE;
    switch (field_size) {
      case 1:
        printf("%s: %hhu\n", argv[2], *((uint8_t*) field_val));
        break;
      case 2:
        printf("%s: %hu\n", argv[2], *((uint16_t*) field_val));
        break;
      case 4:
        printf("%s: %u\n", argv[2], *((uint32_t*) field_val));
        break;
      default:
        printf("Error: size of config field (%zu) is not supported\n", field_size);
        return 2;
    }
  }

  // Set a config field
  // NOTE: We assume that, for simplicity, a config field can be an integer
  // (indeed, it is like that in this configuration). The code below shall be
  // changed on other requirements (e.g. fields that are floats or structs)
  else if (strcmp(argv[1], "set") == 0) {
    if (argc != 4) return 1;

    // Parse the config field to set
    config_field_t f;
    sh_error_on(config_field_id(argv[2], &f) != 0, "Invalid config field", 2);

    // Parse the value for the configuration field
    int value = atoi(argv[3]);

    // Compose the config field setter
    unsigned f_size = config_get_size(f);
    char _f_setter[sizeof(config_setter_t) + f_size];  // Room for the setter
    config_setter_t *f_setter = (config_setter_t*) &_f_setter;

    // Assign the value to the setter
    switch (f_size) {
      case 1:
        *((uint8_t*) f_setter->value) = (uint8_t) value;
        break;
      case 2:
        *((uint16_t*) f_setter->value) = (uint16_t) value;
        break;
      case 4:
        *((uint32_t*) f_setter->value) = (uint32_t) value;
        break;
      default:
        printf("Error: size of config field (%u) is not supported\n", f_size);
        return 2;
    }

    // Send the proper CMD packet
    sh_error_on(
        communication_cmd(st->serial_ctx, CMD_CONFIG_SET_FIELD, f_setter, sizeof(_f_setter)),
        "Could not send CMD packet with config setter", 2);
  }

  else return 1;  // Unknown command, error
  return 0;
}


// CMD: tmon-echo
// Usage: tmon-echo <arg> [arg2 arg3 ...]
// Send a string to the tmon, which should send it back
int tmon_echo(int argc, char *argv[], void *storage) {
  _storage_cast(st, storage);
  if (argc < 2) return 1;
  char str[PACKET_DATA_MAX_SIZE - 1];

  // Reassemble the separated argv[] token into one string
  size_t str_len = 0;
  for (int i=1; i < argc; ++i) {
    size_t tok_len = strlen(argv[i]);
    // TODO: Compact with 'sh_error_on'
    if ((str_len ? str_len + 1 : 0) + tok_len >= PACKET_DATA_MAX_SIZE - 2) {
      fprintf(stderr, "Error: arguments must be at most %d characters long "
          "in total\n", PACKET_DATA_MAX_SIZE - 2);
      return 2;
    }
    if (str_len) str[str_len++] = ' ';
    strcpy(str + str_len, argv[i]);
    str_len += tok_len;
  }
  str[str_len] = '\0';

  // Send the string to the tmon with a CMD_ECHO command
  sh_error_on(communication_cmd(st->serial_ctx, CMD_ECHO, str, str_len + 1) != 0,
      "Could not send the string to echo to the tmon", 3);

  // Get the string back from the tmon
  packet_t pack_rx;
  sh_error_on(communication_recv(st->serial_ctx, &pack_rx),
      "Could not receive the echo string back from the tmon", 3);

  const unsigned char pack_rx_size_expected = PACKET_HEADER_SIZE +
    str_len + sizeof(crc_t);
  const unsigned char pack_rx_size = packet_get_size(&pack_rx);
  if (pack_rx_size != pack_rx_size_expected) { // TODO: Compact
    fprintf(stderr, "Error: The received packet is %hhu bytes long, expected %hhu\n",
        pack_rx_size, pack_rx_size_expected);
    return 2;
  }
  pack_rx.data[str_len] = '\0'; // Substitute first CRC byte with string terminator

  // Print the received string and return
  puts((char*) pack_rx.data);
  return 0;
}


/* TODO: Remove?
// CMD: tmon-debug-recv
// Usage: tmon-debug-recv
// Receive a raw, null-terminated string from the tmon. Used for debug
int tmon_debug_recv(int argc, char *argv[], void *storage) {
  _storage_cast(st, storage);
  if (argc != 1) return 1;

  char *msg = malloc(DEBUG_RECV_MSG_SIZE);
  if (!msg) {
    perror("tmon_debug_recv: Allocator failed");
    return 2;
  }

  int ret = 0;
  if (serial_getstring(st->tmon_fd, msg, DEBUG_RECV_MSG_SIZE) < 0) {
    fputs("Unable to receive raw string from the tmon\n", stderr);
    ret = 2;
  }

  free(msg);
  return ret;
}
*/


// CMD: list
// Usage: list
// List the databases present in the storage
int list(int argc, char *argv[], void *storage) {
  _storage_cast(st, storage);
  if (argc > 1) return 1;

  size_t db_count = list_size(st->dbs);
  printf("DBs present: %zu\n\n", db_count);

  // Print databases metadata
  // TODO: Improve performances using an iterator
  for (size_t i=0; i < db_count; ++i) {
    temperature_db_t *db;
    if (list_get(st->dbs, i, (void**) &db) != 0) {
      printf("Error while fetching database of index %zu\n\n", i);
      return 2;
    }
    printf("Database ID: %u\n%s\nNumber of temperatures: %u\n\n",
        db->id, db->desc, db->size);
  }

  return 0;
}


// CMD: export
// Usage: export <db_id> <output_filepath>
// Export a database (as text, newline-separated float temperatures)
int export(int argc, char *argv[], void *storage) {
  _storage_cast(st, storage);
  if (argc != 3) return 1;
  temperature_db_t *db;

  size_t db_id = atoi(argv[1]);
  if (list_get(st->dbs, db_id, (void**) &db) != 0) {
    fprintf(stderr, "Error: could not fetch database of index %zu\n", db_id);
    return 2;
  }

  if (temperature_db_export(db, argv[2])) {
    fputs("Error while exporting database\n", stderr);
    return 2;
  }

  return 0;
}



// Set of all the shell commands
static shell_command_t _shell_commands[] = {
  (shell_command_t) { // CMD: echo
    .name = "echo",
    .help = "Usage: echo [arg1 arg2 ...]\n"
      "Print back the arguments given",
    .exec = echo
  },

  (shell_command_t) { // CMD: connect
    .name = "connect",
    .help = "Usage: connect <device_path>\n"
      "Connect to an avrtmon, given its device file (usually under /dev)",
    .exec = connect
  },

  (shell_command_t) { // CMD: disconnect
    .name = "disconnect",
    .help = "Usage: disconnect\n"
      "Close an existing connection - Has no effect on the tmon",
    .exec = disconnect
  },

  /*
  (shell_command_t) { // CMD: download
    .name = "download",
    .help = "Usage: download\n"
      "Download all the temperatures from the tmon. creating a new database",
    .exec = download
  },
  */

  (shell_command_t) { // CMD: tmon-reset
    .name = "tmon-reset",
    .help = "Usage: tmon-reset\n"
      "Reset the internal temperatures DB of the tmon",
    .exec = tmon_reset
  },

  (shell_command_t) { // CMD: tmon-config
    .name = "tmon-config",
    .help = "Usage: tmon-config list\n"
            "       tmon-config get <field>\n"
            "       tmon-config set <field> <value>\n"
            "Manipulate the tmon configuration",
    .exec = tmon_config
  },

  (shell_command_t) { // CMD: tmon-echo
    .name = "tmon-echo",
    .help = "Usage: tmon-echo <arg> [arg2 arg3 ...]\n"
            "Send a string to the tmon, which should send it back",
    .exec = tmon_echo
  },

  /*
  (shell_command_t) { // CMD: tmon-debug-recv
    .name = "tmon-debug-recv",
    .help = "Usage: tmon-debug-recv\n"
            "Receive a raw, null-terminated string from the tmon",
    .exec = tmon_debug_recv
  },
  */

  (shell_command_t) { // CMD: list
    .name = "list",
    .help = "Usage: list\n"
      "List the databases present in the storage",
    .exec = list
  },

  (shell_command_t) { // CMD: export
    .name = "export",
    .help = "Usage: export <db_id> <output_filepath>\n"
      "Export a database (as text, newline-separated float temperatures)",
    .exec = export
  }
};

// This is the exposed shell commands set
shell_command_t *shell_commands = _shell_commands;
size_t shell_commands_count = sizeof(_shell_commands) / sizeof(shell_command_t);
