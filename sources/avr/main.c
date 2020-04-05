// avrtmon
// Main routine
// Paolo Lucchesi - Fri 27 Sep 2019 07:18:30 PM CEST
#include <avr/io.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "temperature_daemon.h"
#include "temperature.h"
#include "lmsensor.h"

#include "communication.h"
#include "command.h"

#include "config.h"
#include "buttons.h"
#include "led.h"

#define POWER_ON_LED D22
#define POWER_ACT_LED D24


// Counter for actions performed by the handlers
// Shall be changed to 'volatile' if interrupts will be involved in future
static uint8_t act_perf;


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
  button_enable(btn_start, BTN_VOLT_HIGH);
  button_enable(btn_stop,  BTN_VOLT_HIGH);
}


// Salviamo gli alberi insieme
static inline void power_setup(void) {
  // Disable unneeded modules
  power_usart1_disable();
  power_usart2_disable();
  power_timer0_disable();
  power_timer2_disable();
  power_timer5_disable();
  power_twi_disable();
  power_spi_disable();

  // Power off the built-in LED on pin D13
  DDRB |= 1 << 7;
  PORTB &= ~(1 << 7);

  // Disable digital I/O for analog pins
  DIDR0 = 0xFF;
  DIDR1 = 0xFF;
  DIDR2 = 0xFF;

  // Disable On-Chip Debug module (i.e. JTAG, must be done in 4 cycles to work)
  uint8_t mcucr_old = MCUCR | (1 << JTD);
  MCUCR = 1 << JTD;
  MCUCR = 1 << JTD;
  MCUCR = mcucr_old;
}


int main(int argc, const char *argv[]) {
  // Initialize fundamental modules
  power_setup();
  config_fetch();

  uint8_t debounce_time;
  config_get(CFG_BTN_DEBOUNCE_TIME, &debounce_time);
  button_init(debounce_time);

  temperature_setup(); // Initialize all temperature modules

  // Communication and command stuff
  command_init();
  communication_init();

  led_enable(POWER_ON_LED);
  led_enable(POWER_ACT_LED);
  led_on(POWER_ON_LED);
  led_on(POWER_ACT_LED);
  sei();

  // Main application loop
  while (1) {
    act_perf += communication_handler();      // Check for incoming packets
    act_perf += temperature_daemon_handler(); // Check for new temperatures
    act_perf += button_handler();             // Check for user interaction

    // Enter (interruptable) sleep mode if no action request was performed
    cli();
    if (!act_perf) {
      // Set idle sleep mode (in the docs set_sleep_mode() is... ambiguous)
      SMCR &= ~(1 << SM2 | 1 << SM1 | 1 << SM0);
      led_off(POWER_ACT_LED);
      sleep_enable();
      sei();
      sleep_cpu();
      sleep_disable();
      led_on(POWER_ACT_LED);
    }
    else {
      --act_perf;
      sei();
    }
  }
}
