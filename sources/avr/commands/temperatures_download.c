// avrtmon
// Command interface - Single command template
// Paolo Lucchesi - Wed 23 Oct 2019 04:56:11 PM CEST
// The communication happens as follows:
// 1] [HOST] <CMD> Request to download
// 2] [AVR]  If next (or first) DB is not empty:
//             <CTR> send DB info
// 3] [AVR]  While there are temperatures in the current DB:
//             <DAT> Send temperatures in data bursts (i.e. in bulk)
// 4] [AVR]  If there is another DB, goto [2]
// 5] [AVR]  <CTR> Piggyback CTR packet with no carried data means end of comm.
// TODO: Scheduler fairness
#include "command.h"
#include "temperature.h"
#include "communication.h"
#include "packet.h" // Just packet types

#define COMMAND_NAME cmd_temperatures_download
#define TEMP_BURST (PACKET_DATA_MAX_SIZE / sizeof(temperature_t))
#define MIN(x,y) ((x) > (y) ? (y) : (x))

// Common, side-independent data structure to share DB informations
static temperature_db_info_t db_info;

// Keep track of the download state across different received packets
static uint8_t temp_db_id;
static temperature_id_t temp_idx = 0, temp_count;
static temperature_t temp_buf[TEMP_BURST];


// Change DB currently in use
// Returns 0 on success, 1 if the DB does not exist
static uint8_t _change_current_db(uint8_t db_id) {
  if (temperature_db_info(db_id, db_info) != 0) {
    communication_craft_and_send(PACKET_TYPE_CTR, NULL, 0);
    return 1;
  }

  temp_idx = 0;
  temperature_db_info_extract(db_info, &temp_db_id, &temp_count);
  communication_craft_and_send(PACKET_TYPE_CTR, db_info, sizeof(db_info));
  return 0;
}


// Single command iteration of the temperature uploader
static uint8_t _iterate(const void *arg) {
  if (temp_idx == temp_count && _change_current_db(temp_db_id+1) != 0)
    return CMD_RET_FINISHED;

  temperature_id_t to_get = MIN(temp_count-temp_idx, TEMP_BURST);
  temperature_get_bulk(temp_db_id, temp_idx, to_get, temp_buf);
  communication_craft_and_send(PACKET_TYPE_DAT,
      (void*) temp_buf, to_get * sizeof(temperature_t));
  temp_idx += to_get;
  return CMD_RET_ONGOING;
}


// Command starter
static uint8_t _start(const void *arg) {
  if (_change_current_db(0) != 0)
    return CMD_RET_FINISHED;
  return CMD_RET_ONGOING;
}


static command_t _cmd = {
  .start   = _start,
  .iterate = _iterate,
  .opmode  = NULL
};

command_t *COMMAND_NAME = &_cmd;
