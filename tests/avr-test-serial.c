// AVR Temperature Monitor -- Paolo Lucchesi
// Serial communication layer - Test Unit
// This AVR-side program expects to receive all the lowercase of the latin
// alphabet, and sends back those letters in uppercase.
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "serial.h"

#define D13_MASK (1 << 7)
#define READY_MSG "Ready to receive\n"

#define BUF_RX_SIZE 256
uint8_t buf_rx[BUF_RX_SIZE];
uint8_t buf_tx[1];


int main(int argc, const char *argv[]) {
  serial_init();
  DDRB |= D13_MASK;
  sei();

  // Send READY_MSG without trailing '\0'
  serial_tx(READY_MSG, sizeof(READY_MSG) - 1);
  while (serial_tx_ongoing())
    ;

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

  // Wait for AVR board physical reset
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  while (1)
    sleep_cpu();
}
