// avrtmon
// Main routine
// Paolo Lucchesi - Fri 27 Sep 2019 07:18:30 PM CEST
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>


#include "lmsensor_timer.h"
#include "communication.h"
#include "temperature.h"
#include "lmsensor.h"
#include "command.h"
#include "buttons.h"
#include "config.h"


// Testing functions
static inline void led_blink(void) {
  PORTB |= 1 << 7;
  _delay_ms(250);
  PORTB &= ~(1 << 7);
}

static inline void led_init(void) {
  DDRB |= (1 << 7);
}


// Temperature handler
extern void temperature_handler(void);


int main(int argc, const char *argv[]) {
  // Initialize all modules
  //config_fetch();
  //temperature_init();
  command_init();
  //buttons_init();
  com_init();
  led_init();

  sei();

  // Main application loop
  while (1) {
    led_blink();
    com_handler();          // Check for incoming packets
    //temperature_handler();  // Check for temperatures ready to be registered
    //buttons_handler();      // Check for AVR-side user interaction
  }
}
