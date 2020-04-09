// AVR Temperature Monitor -- Paolo Lucchesi
// Serial communication layer - Test Unit
// This AVR-side program expects to receive all the lowercase of the latin
// alphabet, and sends back those letters in uppercase.
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "serial.h"

#define D13_MASK (1 << 7)
#define READY_MSG "Ready to receive\n"

#define BUF_RX_SIZE 52
volatile uint8_t buf_rx[BUF_RX_SIZE];
uint8_t buf_tx[1];


int main(int argc, const char *argv[]) {
  serial_init();
  DDRB |= D13_MASK;
  sei();

  /*
  // Send READY_MSG without trailing '\0'
  serial_tx(READY_MSG, sizeof(READY_MSG) - 1);
  while (serial_tx_ongoing())
    ;
  */


  /* Version 2 */

  // Receive start byte (every byte will do)
  while (!serial_rx_getchar(buf_rx)) ;

  // Transmit every byte in the interval [0x00,0xFE]
  uint16_t sent=0;
  for (uint8_t c=0; c < 0xFF; ++c)
    if (serial_tx(&c, 1) == 0) ++sent;

  // Transmit 0xFF
  static const uint8_t last = 0xFF;
  if (serial_tx(&last, 1) == 0) ++sent;

  // Transmit 3 times 0x00
  static const uint8_t zero = 0x00;
  for (uint8_t i=0; i < 3; ++i)
    serial_tx(&zero, 1);

  // Transmit 'sent'
  while (serial_tx(&sent, 2) != 0) ;

  while (1) ;

  /* Version 1 */

  /*
  uint8_t chars[26];
  for (uint8_t i=0; i < 26; ++i)
    chars[i] = i + 0x41;  // All capital letters from 'A' to 'Z'

  while (1) {
    PORTB = D13_MASK;

    for (uint8_t i=0; i < BUF_RX_SIZE; ) {
      if (!serial_rx_available()) {
        _delay_ms(500);
        continue;
      }

      serial_rx_getchar(buf_rx + i);
      buf_tx[0] = chars[(buf_rx[i] - 0x61) % 26];
      serial_tx(buf_tx, 1);
      while (serial_tx_ongoing())
        ;
      ++i;
    }

    PORTB = 0;
    _delay_ms(2000);
  }
  */
}
