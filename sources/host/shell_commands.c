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


// Type definition for the internal shell storage
typedef struct _shell_storage_s {
  int tmon_fd;
  list_t *dbs;
} shell_storage_t;

// Declare a shell_storage_t variable casted from an opaque pointer
#define _storage_cast(st,opaque) shell_storage_t *st=(shell_storage_t*)(opaque)

// Allocate and initialize a shell storage
// Returns an opaque pointer to the allocated storage, or NULL on failure
void *shell_storage_new(void) {
  shell_storage_t *s = malloc(sizeof(shell_storage_t));
  if (!s) return NULL;

  s->dbs = list_new();
  if (!s->dbs) {
    free(s);
    return NULL;
  }

  return (void*) s;
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
int connect(int argc, char *argv[], void *storage) {
  _storage_cast(st, storage);
  if (argc < 2) return 1;
  if (st->tmon_fd >= 0) return 2; // i.e. already connected

  int tmon_fd = serial_open(argv[1]);
  if (tmon_fd < 0) return 2;

  st->tmon_fd = tmon_fd;
  return serial_craft_and_send(PACKET_TYPE_HND, NULL, 0, st->tmon_fd);
}


// CMD: reconnect
// Usage: reconnect
// Reset existing communication session between host and tmon
int reconnect(int argc, char *argv[], void *storage) {
  _storage_cast(st, storage);
  if (argc != 1) return 1;
  if (st->tmon_fd < 0) return 2;
  return serial_craft_and_send(PACKET_TYPE_HND, NULL, 0, st->tmon_fd);
}


// CMD: download
// Usage: download
// Download all the temperatures from the tmon. creating a new database
int download(int argc, char *argv[], void *storage) {
  _storage_cast(st, storage);
  if (argc > 1) return 1;
  if (st->tmon_fd < 0) return 2;

  // Max number of temperatures contained in a DB
  static const unsigned temp_burst = PACKET_DATA_MAX_SIZE / sizeof(uint16_t);

  // Store the ID for the new database
  static size_t db_id = 0;

  // Send a download command to the tmon
  serial_cmd(CMD_TEMPERATURES_DOWNLOAD, NULL, 0, st->tmon_fd);

  // Get the number of temperature from the tmon response (as temp_count)
  packet_t pack_rx;
  serial_recv(st->tmon_fd, &pack_rx);
  command_payload_t *payload_in = (command_payload_t*) pack_rx.data;
  command_download_arg_t temp_count =
    *((command_download_arg_t*)(payload_in->arg));

  if (temp_count) {
    // Create a new database for the incoming temperatures
    temperature_db_t *db = temperature_db_new(db_id, temp_count, NULL);

    // Get and add the temperatures
    unsigned i = 0;
    while (i < temp_count) {
      serial_recv(st->tmon_fd, &pack_rx);
      do {
        temperature_register(db, i++,  // Note: increment of the temperature idx
            temperature_raw2float(pack_rx.data[i % temp_burst]));
      } while (i % temp_burst && i < temp_count);
    }

    // Add the database to the global storage
    list_add(st->dbs, db);
  }

  // Get the last CTR packet 
  serial_recv(st->tmon_fd, &pack_rx);

  return 0;
}


// CMD: tmon-reset
// Usage: tmon-reset
// Reset the internal temperatures DB of the tmon
int tmon_reset(int argc, char *argv[], void *storage) {
  _storage_cast(st, storage);
  if (argc != 1) return 1;
  if (st->tmon_fd < 0) return 2;
  return serial_cmd(CMD_TEMPERATURES_RESET, NULL, 0, st->tmon_fd);
}


// CMD: tmon-config
// Usage: tmon-config list
//        tmon-config get <field>
//        tmon-config set <field> <value>
// Manipulate the tmon configuration
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
    if (config_field_id(argv[2], &f) != 0) {
      printf("Error: '%s' is not a valid configuration field\n", argv[2]);
      return 2;
    }

    // Send a CONFIG_GET_FIELD command to the tmon
    serial_cmd(CMD_CONFIG_GET_FIELD, &f, sizeof(config_field_t), st->tmon_fd);

    // Retrieve the field from the tmon
    packet_t pack_rx;
    serial_recv(st->tmon_fd, &pack_rx);

    // Handle the received config field value
    // We assume that every config field is an integer
    void *field_val = (void*) pack_rx.data;
    size_t field_size = pack_rx.size - PACKET_HEADER_SIZE;
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
    if (config_field_id(argv[2], &f) != 0) {
      printf("Error: '%s' is not a valid configuration field\n", argv[2]);
      return 2;
    }

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
    serial_cmd(CMD_CONFIG_SET_FIELD, f_setter, sizeof(_f_setter), st->tmon_fd);
  }

  else return 1;  // Unknown command, error
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
  if (list_get(st->dbs, db_id, (void**) &db) != 0)
    return 2;

  if (temperature_db_export(db, argv[2]))
    return 2;

  return 0;
}



// Expose an array of all the commands to the global scope
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

  (shell_command_t) { // CMD: reconnect
    .name = "reconnect",
    .help = "Usage: reconnect\n"
      "Reset existing communication session between host and tmon",
    .exec = reconnect
  },

  (shell_command_t) { // CMD: download
    .name = "download",
    .help = "Usage: download\n"
      "Download all the temperatures from the tmon. creating a new database",
    .exec = download
  },

  (shell_command_t) { // CMD: tmon-reset
    .name = "tmon",
    .help = "Usage: tmon-reset\n"
      "Reset the internal temperatures DB of the tmon",
    .exec = tmon_reset
  },

  (shell_command_t) { // CMD: tmon-config
    .name = "tmon",
    .help = "Usage: tmon-config list\n"
            "       tmon-config get <field>"
            "       tmon-config set <field> <value>"
            "Manipulate the tmon configuration",
    .exec = tmon_config
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

// This is the exposed one
shell_command_t *shell_commands = _shell_commands;
size_t shell_commands_count = sizeof(_shell_commands) / sizeof(shell_command_t);
