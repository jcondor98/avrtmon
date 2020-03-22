// avrtmon
// Test Unit on data structures metadata et similia - AVR side
// Paolo Lucchesi - Tue 17 Dec 2019 12:31:34 AM CET
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "serial.h"
#include "packet.h"
#include "led.h"
//#include "command.h"
//#include "communication.h"
//#include "lmsensor.h"
//#include "temperature.h"

#define MSG_MAX_LEN 255
char msg[MSG_MAX_LEN];


// For now, only packet and command structures are tested
// TODO: Test other stuff
int main(int argc, const char *argv[]) {
  serial_init();
  led_init();
  sei();

  uint8_t msg_len = snprintf(msg, MSG_MAX_LEN, "Metadata for packet_t:\n"
      "sizeof packet_t -> %u\n"
      "[ Ignoring address for bitfields ]\n"
      "offsetof .data -> %u\n",
      sizeof(packet_t), offsetof(packet_t, data));

  led_blink(200, 10);

  ++msg_len; // Include terminating null byte
  for (uint8_t burst=0; burst < msg_len/TX_BUFFER_SIZE + 1; ++burst) {
    serial_tx(msg + burst * TX_BUFFER_SIZE, TX_BUFFER_SIZE); // TODO: Last burst
    while (serial_tx_ongoing())
      ;
  }
}
