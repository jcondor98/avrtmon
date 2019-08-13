// avrtmon
// Serial communication layer - Source file
// Paolo Lucchesi - Sun 11 Aug 2019 03:26:17 PM CEST
#include "serial.h"
#include <avr/io.h>
#include <util/delay.h>


int main(int argc, const char *argv[]) {
  serial_init();

  volatile uint8_t chars[24];
  for (uint8_t i=0; i < 24; ++i)
    chars[i] = i + 0x41;  // All capital letters from 'A' to 'Z'

  volatile uint8_t buf_rx[8];
  uint8_t buf_tx[8];

  DDRB |= (1 << 7);
  while (1) {
    PORTB ^= (1 << 7);
    serial_rx(buf_rx, 8);

    while (serial_rx_ongoing())
      ;

    PORTB ^= (1 << 7);
    for (int i=0; i < 8; ++i)
      buf_tx[i] = chars[(buf_rx[i] - 0x61) % 24];

    serial_tx(buf_tx, 8);

    while (serial_tx_ongoing())
      ;
  }
}


/*
int main(int argc, const char *argv[]) {
  serial_init();
  for (uint8_t i=0; 1; i = (i+1) % 2) {
    _delay_ms(20);
    serial_send(i ? s1 : s2, s_len);
    while (serial_tx_sent() < s_len)
      ;
  }
}
*/
