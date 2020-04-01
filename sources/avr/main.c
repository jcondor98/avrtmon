// avrtmon
// Main routine
// Paolo Lucchesi - Fri 27 Sep 2019 07:18:30 PM CEST
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "temperature_daemon.h"
#include "temperature.h"

#include "communication.h"
#include "command.h"

#include "config.h"
#include "buttons.h"
#include "led.h"

#define POWER_ON_LED D22


// Initialize stuff related to the temperature modules
static inline void temperature_setup(void) {
  uint16_t resolution, interval;
  config_get(CFG_TEMPERATURE_TIMER_RESOLUTION, &resolution);
  config_get(CFG_TEMPERATURE_TIMER_INTERVAL,   &interval);

  uint8_t btn_start, btn_stop;
  config_get(CFG_START_PIN, &btn_start);
  config_get(CFG_STOP_PIN,  &btn_stop);

  uint8_t lm_pin;
  config_get(CFG_LMSENSOR_PIN, &lm_pin);

  temperature_init();
  temperature_daemon_init(resolution, interval, lm_pin);
  button_action_set(btn_start, temperature_daemon_start);
  button_action_set(btn_stop,  temperature_daemon_stop);
  button_enable(btn_start);
  button_enable(btn_stop);
}


int main(int argc, const char *argv[]) {
  // Initialize fundamental modules
  config_fetch();
  button_init();

  temperature_setup(); // Initialize all temperature modules

  // Communication and command stuff
  command_init();
  communication_init();

  led_enable(POWER_ON_LED);
  led_on(POWER_ON_LED);
  sei();

  // Main application loop
  while (1) {
    communication_handler();      // Check for incoming packets
    temperature_daemon_handler(); // Check for new temperatures to register
    button_handler();             // Check for AVR-side user interaction

    _delay_ms(200); // TODO: Pause
  }
}
