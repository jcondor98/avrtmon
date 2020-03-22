// avrtmon
// Debug LED utility - Head file
// Paolo Lucchesi - Wed 18 Dec 2019 01:45:01 AM CET
#ifndef __LED_H
#define __LED_H
#include <avr/io.h>

#define LED_MASK (1<<7)  // i.e. Digital Pin 13

// Initialize the LED interface
#define led_init() do {\
  DDRB |= LED_MASK;\
  PORTB &= ~(LED_MASK);\
} while (0)

// Blink the chosen LED
#define led_blink(interval_ms, times) do {\
  for (uint8_t i=0; i < times; ++i) {\
    PORTB |= LED_MASK;\
    _delay_ms(interval_ms/2);\
    PORTB &= ~(LED_MASK);\
    _delay_ms(interval_ms/2);\
  }\
} while (0)


#endif    // __LED_H
