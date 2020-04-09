// AVR Temperature Monitor -- Paolo Lucchesi
// LED Interface - Head file
#ifndef _LED_INTERFACE_H
#define _LED_INTERFACE_H
#include <avr/io.h>

// Identifiers for different LEDs
// Pins of PORTA (i.e. Digital[22,29]) are used
typedef enum LED_PIN_E {
  D22 = 1 << 0, D23 = 1 << 1, D24 = 1 << 2, D25 = 1 << 3,
  D26 = 1 << 4, D27 = 1 << 5, D28 = 1 << 6, D29 = 1 << 7
} led_pin_t;

// Enable LEDs (enables internal pull-up resistors)
// (e.g. if ure using D23 and D24, call led_init(D23 | D24)
#define led_enable(leds) do { DDRA |= (leds); PORTA &= ~(leds); } while (0)

// Turn LEDs on/off
#define led_on(leds)  do { PORTA |=  (leds); } while (0)
#define led_off(leds) do { PORTA &= ~(leds); } while (0)

#endif  // _LED_INTERFACE_H
