// avrtmon
// Serial communication layer - Source file
// Paolo Lucchesi - Sun 11 Aug 2019 03:26:17 PM CEST
#include "serial.h"
#include <util/delay.h>
#include <avr/io.h>


int main(int argc, const char *argv[]) {
  serial_init();

  uint8_t chars[0x5A - 0x41];
  for (uint8_t i=0; i < 24; ++i)
    chars[i] = i + 0x41;  // All capital letters from 'A' to 'Z'

  volatile uint8_t buf_rx[8];
  uint8_t buf_tx[8];

  DDRB |= 1 << 7;
  while (1) {
    PORTB ^= 1 << 7;
    serial_rx_unlock();
    serial_tx_lock();
    serial_recv(buf_rx, 8);

    while (serial_rx_available() != 8) ;

    serial_rx_lock();

    PORTB ^= 1 << 7;
    for (int i=0; i < 8; ++i)
      buf_tx[i] = chars[buf_rx[i % sizeof(chars)]];

    serial_tx_unlock();
    serial_send(buf_tx, 8);

    while (serial_tx_sent() != 8) ;
  }
}
