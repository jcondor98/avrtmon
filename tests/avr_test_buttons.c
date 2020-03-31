// avrtmon
// Buttons interface test unit
// Paolo Lucchesi - Sun 29 Mar 2020 10:49:43 PM CEST
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "serial.h"
#include "buttons.h"


const char d52_fmt[] = "Button on D52 pressed %hhu times\n";
const char d53_fmt[] = "Button on D53 pressed %hhu times\n";
char tx_buf[sizeof(d52_fmt) + 10];

void d52_callback(uint8_t pressed) {
  uint8_t to_write = snprintf(tx_buf, sizeof(tx_buf), d52_fmt, pressed);
  serial_tx(tx_buf, to_write);
}

void d53_callback(uint8_t pressed) {
  uint8_t to_write = snprintf(tx_buf, sizeof(tx_buf), d53_fmt, pressed);
  serial_tx(tx_buf, to_write);
}


int main(int argc, const char *argv[]) {
  serial_init();

  button_init();
  button_action_set(D50, d52_callback);
  button_action_set(D53, d53_callback);
  button_enable(D50);
  button_enable(D53);

  sei();

  while (1) {
    button_handler();
    _delay_ms(2000);
    uint8_t to_write = snprintf(tx_buf, sizeof(tx_buf), "Value of PINB is %hhx\n", PINB);
    serial_tx(tx_buf, to_write);
  }
}
