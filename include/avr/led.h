// avrtmon
// LED Interface - Head file
// Paolo Lucchesi - Wed 18 Dec 2019 01:45:01 AM CET
#ifndef _LED_INTERFACE_H
#define _LED_INTERFACE_H
#include <avr/io.h>

// Identifiers for different LEDs
// Pins of PORTA (i.e. Digital[22,29]) are used
typedef enum LED_PIN_E {
  D22 = 0, D23, D24, D25, D26, D27, D28, D29
} led_pin_t;

// Initialize the LED module, specifying which LEDs will be used
// (e.g. if ure using D23 and D24, call led_init(D23 | D24)
void led_init(uint8_t led_mask);

// Turn LEDs on/off
// Return 0 on success, 1 if the LED does not exist
void led_on(uint8_t led_id);
void led_off(uint8_t led_id);

// TODO: Add blinking?

#endif  // _LED_INTERFACE_H
