// avrtmon
// LED Interface - Head file
// Paolo Lucchesi - Tue 24 Mar 2020 05:54:05 PM CET
#include <avr/io.h>
#include "led.h"

// Initialize the LED module, specifying which LEDs will be used
// (e.g. if ure using D23 and D24, call led_init(D23 | D24)
void led_enable(uint8_t led_mask) {
  DDRA  |=  led_mask;
  PORTA &= ~led_mask; // LEDs off
}

// Turn LEDs on/off -- You can switch multiple LEDs (e.g. led_on(D27 | D26))
void led_on(uint8_t led_mask)  { PORTA |= led_mask; }
void led_off(uint8_t led_mask) { PORTA &= ~(led_mask); }
