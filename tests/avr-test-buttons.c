// AVR Temperature Monitor -- Paolo Lucchesi
// Buttons interface - Test Unit
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "serial.h"
#include "buttons.h"


const char d52_msg[] = "Button on D52 pressed\n";
const char d53_msg[] = "Button on D53 pressed\n";
const char d21_msg[] = "Button on D21 pressed\n";

void d52_callback(uint8_t pressed) {
  serial_tx(d52_msg, sizeof(d52_msg) - 1);
}

void d53_callback(uint8_t pressed) {
  serial_tx(d53_msg, sizeof(d53_msg) - 1);
}

void d21_callback(uint8_t pressed) {
  serial_tx(d21_msg, sizeof(d21_msg) - 1);
}


int main(int argc, const char *argv[]) {
  // Initialize modules
  serial_init();
  button_init(50);

  // Setup buttons
  button_action_set(D21, d21_callback);
  button_action_set(D52, d52_callback);
  button_action_set(D53, d53_callback);
  button_enable(D21, BTN_VOLT_HIGH);
  button_enable(D52, BTN_VOLT_HIGH);
  button_enable(D53, BTN_VOLT_HIGH);

  sei();

  // Main application loop
  while (1) button_handler();
}
