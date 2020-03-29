// avrtmon
// Program shell (i.e. command line) - Commands
// Paolo Lucchesi - Tue 05 Nov 2019 01:36:25 AM CET
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "shell.h"
#include "list.h"
#include "serial.h"
#include "temperature.h"
#include "communication.h"


// Declare a shell_storage_t variable casted from an opaque pointer
#define _storage_cast(st,opaque) shell_storage_t *st=(shell_storage_t*)(opaque)

// Handier function to send and receive packets
#define SERIAL_CTX (st->serial_ctx) // Must be the context in EVERY function below
#define psend(type,data,data_size)\
  communication_craft_and_send(SERIAL_CTX,type,data,data_size)
#define precv(p) communication_recv(SERIAL_CTX,p)
#define pcmd(cmd,arg,arg_size) communication_cmd(SERIAL_CTX,cmd,arg,arg_size)

// Type definition for the internal shell storage
typedef struct _shell_storage_s {
  serial_context_t *serial_ctx;
  list_t *dbs;
} shell_storage_t;

// Wrapper to destroy DBs when destroying 'dbs'
static void _temperature_db_item_destroyer(void *db) {
  temperature_db_delete(db);
}


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

  SERIAL_CTX = NULL;  // i.e. not connected
  return (void*) st;
}

// Cleanup this shell environment (i.e. free memory, close descriptors...)
// Also frees the shell storage data structure itself
void shell_cleanup(shell_t *s) {
  if (!s) return;
  _storage_cast(st, s->storage);

  // Close the connection with the tmon, if any
  if (SERIAL_CTX && serial_close(SERIAL_CTX) != 0)
    fputs("Error: Could not close the tmon file descriptor\n", stderr);

  // Free the storage
  list_delete(st->dbs, _temperature_db_item_destroyer);
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

  if (SERIAL_CTX)
    fputs("Open serial context found; reconnecting\n", stderr);
  else { // Open a descriptor for the tmon
    SERIAL_CTX = serial_open(argv[1]);
    sh_error_on(!SERIAL_CTX, "Unable to connect to the tmon", 2);
  }

  // Estabilish the connection
  sh_error_on(communication_connect(SERIAL_CTX) != 0,
      "Could not send a handshake packet", 3);

  return 0;
}


// CMD: disconnect
// Usage: disconnect
// Close an existing connection - Has no effect on the tmon
int disconnect(int argc, char *argv[], void *storage) {
  _storage_cast(st, storage);
  if (argc != 1) return 1;
  sh_error_on(!SERIAL_CTX, "tmon is not connected", 2);

  int ret = serial_close(SERIAL_CTX);
  SERIAL_CTX = NULL; // i.e. set to disconnected
  sh_error_on(ret != 0, "Could not close the tmon file descriptor", 3);

  return 0;
}


// CMD: download
// Usage: download
// Download all the temperatures from the tmon, creating a new database
// The communication happens as follows:
// 1] [HOST] <CMD> Request to download
// 2] [AVR]  If next (or first) DB is not empty:
//             <CTR> send DB info
// 3] [AVR]  While there are temperatures in the current DB:
//             <DAT> Send temperatures in data bursts (i.e. in bulk)
// 4] [AVR]  If there is another DB, goto [2]
// 5] [AVR]  <CTR> Piggyback CTR packet with no carried data means end of comm.
int download(int argc, char *argv[], void *storage) {
  _storage_cast(st, storage);
  if (argc > 1) return 1;
  sh_error_on(!SERIAL_CTX, "tmon is not connected", 2);

  // Store DBs in a list
  list_t *db_list = list_new();
  sh_error_on(!db_list, "Could not create new linked list", 2);

  // DB which is currently in reception, with variables to store metadata
  // TODO: Initialize 'db_id' to avoid conflicts
  uint8_t db_id;
  uint16_t db_reg_resolution, db_reg_interval;
  temperature_id_t db_size;
  temperature_db_t *db_current = NULL; // TODO: Not necessary to initialize if first packet is asserted to be CTR

  // Buffer to received packets
  packet_t pack_rx[1];


  // Send a download command to the tmon
  sh_error_on(pcmd(CMD_TEMPERATURES_DOWNLOAD, NULL, 0) != 0,
      "Could not send CMD packet", 3);

  // TODO: First packet must be a CTR one

  // Main downloader loop
  while (1) {
    precv(pack_rx);
    unsigned char type = packet_get_type(pack_rx);
    unsigned char data_size = packet_data_size(pack_rx);

    // New database incoming
    if (type == PACKET_TYPE_CTR) {
      list_add(db_list, db_current);

      if (data_size == 0) { // No more data to receive
        list_concat(st->dbs, db_list);
        return 0;
      }

      else if (data_size == sizeof(temperature_db_info_t)) { // New DB incoming
        temperature_db_info_extract(pack_rx->data, &db_id, &db_size,
            &db_reg_resolution, &db_reg_interval);
        db_current = temperature_db_new(db_id, db_size,
            db_reg_resolution, db_reg_interval, NULL);
      }

      else break; // Error: unexpected packet data size
    }

    // New temperatures incoming -- TODO: Check DB size
    else if (type == PACKET_TYPE_DAT) {
      const unsigned burst = data_size / sizeof(temperature_db_t);
      temperature_register_bulk(db_current, burst, (float*) pack_rx->data);
    }

    else break; // Error: unexpected packet type
  }


  // Error handler
  list_delete(db_list, _temperature_db_item_destroyer);
  sh_error_on(1, "Communication failure", 3); // Communication error
}


// CMD: tmon-reset
// Usage: tmon-reset
// Reset the internal temperatures DB of the tmon
int tmon_reset(int argc, char *argv[], void *storage) {
  _storage_cast(st, storage);
  if (argc != 1) return 1;

  sh_error_on(!SERIAL_CTX, "tmon is not connected", 2);
  sh_error_on(pcmd(CMD_TEMPERATURES_RESET, NULL, 0) != 0,
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
    sh_error_on(pcmd(CMD_CONFIG_GET_FIELD, &f, sizeof(config_field_t)),
        "Could not send CMD packet", 3);

    // Retrieve the field from the tmon
    packet_t pack_rx[1];
    sh_error_on(precv(pack_rx), "Could not receive config field value", 3);

    // Handle the received config field value
    // We assume that every config field is an integer
    void *field_val = (void*) pack_rx->data;
    size_t field_size = packet_data_size(pack_rx);
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
    sh_error_on(pcmd(CMD_CONFIG_SET_FIELD, f_setter, sizeof(_f_setter)),
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
  sh_error_on(pcmd(CMD_ECHO, str, str_len + 1) != 0,
      "Could not send the string to echo to the tmon", 3);

  // Get the string back from the tmon
  packet_t pack_rx;
  sh_error_on(precv(&pack_rx),
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

  (shell_command_t) { // CMD: download
    .name = "download",
    .help = "Usage: download\n"
      "Download all the temperatures from the tmon. creating a new database",
    .exec = download
  },

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
