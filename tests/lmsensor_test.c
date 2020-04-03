// avrtmon
// LM Sensor layer - Unit Test
// Paolo Lucchesi - Tue 20 Aug 2019 04:20:08 PM CEST
// This test unit wait for a character coming from the host (i.e. PC) through
// the serial interface as a handshake. Any byte will do.
// After that, the AVR board will periodically register a temperature, sending
// it as a uint16_t (little-endian) representing the raw tension got by the ADC
#include <avr/io.h>
#include <util/delay.h>
#include "lmsensor.h"
#include "serial.h"


#define D13_MASK (1 << 7)

int main(int argc, const char *argv[]) {
  lm_init();
  serial_init();
  DDRB = D13_MASK;
  PORTB = D13_MASK; // Turned on LED means "waiting for host handshake"

  volatile uint8_t rx_buf[32];
  uint16_t temperature;

  // Wait for a "handshake" coming from the host
  // Basically, wait for a byte to start the conversion
  serial_rx(rx_buf, 1);
  while (serial_rx_ongoing())
    ;

  // Start test unit
  PORTA = 0;
  PORTB = 0; // Turn off LED
  while (1) {
    // Get a temperature
    lm_convert();
    while (! lm_available())
      ;
    temperature = lm_getresult();

    // Flash the D13 LED
    PORTB = D13_MASK;
    _delay_ms(100);
    PORTB = 0;

    // Transmit the temperature - LITTLE ENDIAN
    serial_tx((uint8_t*)(&temperature), sizeof(uint16_t));
    while (serial_tx_ongoing())
      ;

    // Wait for a reasonable interval
    _delay_ms(1400);
  }

  return 0; // Dead code
}

