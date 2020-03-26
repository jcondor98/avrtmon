// avrtmon
// Main routine
// Paolo Lucchesi - Fri 27 Sep 2019 07:18:30 PM CEST
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>


//#include "lmsensor_timer.h"
#include "communication.h"
//#include "temperature.h"
//#include "lmsensor.h"
#include "command.h"
//#include "buttons.h"
//#include "config.h"
#include "led.h"


// Temperature handler
//extern void temperature_handler(void);


int main(int argc, const char *argv[]) {
  // Initialize all modules
  //config_fetch();
  //temperature_init();
  command_init();
  //buttons_init();
  communication_init();
  led_init(0x00); // TODO: Use some LEDs

  sei();

  // Main application loop
  while (1) {
    communication_handler();  // Check for incoming packets
    //temperature_handler();  // Check for temperatures ready to be registered
    //buttons_handler();      // Check for AVR-side user interaction

    _delay_ms(200); // TODO: Pause
  }
}
