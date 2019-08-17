// avrtmon
// Serial communication layer - Unit test
// Paolo Lucchesi - Sun 11 Aug 2019 03:26:17 PM CEST
#include "serial.h"
#include <avr/io.h>
#include <util/delay.h>

#define D13_MASK (1 << 7)

#define BUF_RX_SIZE 52
volatile uint8_t buf_rx[BUF_RX_SIZE];
uint8_t buf_tx[1];


int main(int argc, const char *argv[]) {
  serial_init();
  DDRB |= D13_MASK;

  uint8_t chars[26];
  for (uint8_t i=0; i < 26; ++i)
    chars[i] = i + 0x41;  // All capital letters from 'A' to 'Z'

  while (1) {
    PORTB = D13_MASK;
    serial_rx(buf_rx, BUF_RX_SIZE);
    for (uint8_t i=0; i < BUF_RX_SIZE; ) {
      uint8_t received = serial_rx_available();
      if (i < received) {
        buf_tx[0] = chars[(buf_rx[i] - 0x61) % 26];
        serial_tx(buf_tx, 1);
        while (serial_tx_ongoing())
          ;
        ++i;
      }
    }
    PORTB = 0;
    _delay_ms(2000);
  }
}
