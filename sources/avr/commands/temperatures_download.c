// avrtmon
// Command interface - Single command template
// Paolo Lucchesi - Wed 23 Oct 2019 04:56:11 PM CEST
#include "command.h"
#include "temperature.h"
#include "communication.h"

#define COMMAND_NAME cmd_temperatures_download
#define TEMP_BURST (PACKET_DATA_MAX_SIZE / sizeof(temperature_t))

// Keep track of the download state across different received packets
static id_t temp_idx = 0, temp_total;
static temperature_t temp_buf[TEMP_BURST];


// Command starter
static void _start(const void *arg) {
  // Initialize command status variables
  temp_idx = 0;
  temp_total = temperature_count();

  // Send the number of temperatures present in the DB
  id_t temp_count = temperature_count();
  com_craft_and_send(PACKET_TYPE_CTR, (void*)(&temp_count), sizeof(id_t));
}


// Define the opmode for the download command
// Operation for PACKET_TYPE_ACK
// Last sent temperature burst was correctly received, so send another one
static void _op_ack(const packet_t *rx_pack) {
  if (temp_idx >= temp_total) {
    // Communicate end of transmission with an empty CTR packet
    com_craft_and_send(PACKET_TYPE_CTR, NULL, 0);
    temperature_db_reset();
    return;
  }

  // TODO: Check the indexes
  id_t i;
  for (i=temp_idx; i < temp_idx + TEMP_BURST && i < temp_total; ++i)
    temperature_get(i, temp_buf + (i % TEMP_BURST)); // Fetch the temperature
  temp_idx = i;  // Update the global temperature index
}

// The opmode itself
static com_operation_f _opmode[] = {
  NULL, _op_ack, NULL, NULL, NULL, NULL
};


static command_t _cmd = {
  .start = _start,
  .opmode = _opmode
};

command_t *COMMAND_NAME = &_cmd;
