// AVR Temperature Monitor -- Paolo Lucchesi
// Main routine
#include <avr/io.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "sleep_util.h"

#include "temperature_daemon.h"
#include "temperature.h"
#include "lmsensor.h"

#include "communication.h"
#include "command.h"

#include "config.h"
#include "buttons.h"
#include "led.h"

#define POWER_ON_LED D22


// Perform setup routine?
static uint8_t perform_setup = 1;

// Counter for actions performed by the handlers
// Shall be changed to 'volatile' if interrupts will be involved in future
static uint8_t act_perf;

// Buttons, dynamically loaded from the configuration
static uint8_t btn_start, btn_stop, btn_poweroff;


// Poweroff the AVR board (with capability to resume) -- Buttons-friendly
static inline void poweroff(uint8_t pressed) {
  // Clean before shutting down
  temperature_daemon_stop(1);
  button_disable(btn_start);
  button_disable(btn_stop);
  led_off(POWER_ON_LED);
  led_off(POWER_ACT_LED);

  // Good night (go to sleep)
  SMCR |= 1 << SM1;
  SMCR &= ~(1 << SM2 | 1 << SM0);
  sleep_enable();
  sleep_cpu();

  // Good morning (wake up)
  sleep_disable();
  cli(); // No interrupts while setting up again
  perform_setup = 1; // Repeat setup routine
}


// Initialize stuff related to the temperature modules
static inline void temperature_setup(void) {
  // Get temperature timing parameters
  uint16_t resolution, interval;
  config_get(CFG_TEMPERATURE_TIMER_RESOLUTION, &resolution);
  config_get(CFG_TEMPERATURE_TIMER_INTERVAL,   &interval);

  // Get ADC/LM35 parameters
  uint8_t lm_pin;
  config_get(CFG_LMSENSOR_PIN, &lm_pin);

  // Initialize temperature modules
  temperature_init();
  temperature_daemon_init(resolution, interval, lm_pin);
}


// Initialize and setup buttons
static inline void buttons_setup(void) {
  // Initalize buttons module
  uint8_t debounce_time;
  config_get(CFG_BTN_DEBOUNCE_TIME, &debounce_time);
  button_init(debounce_time);

  // Get buttons pins
  config_get(CFG_POWEROFF_PIN, &btn_poweroff);
  config_get(CFG_START_PIN,    &btn_start);
  config_get(CFG_STOP_PIN,     &btn_stop);
  
  // Set buttons callbacks
  button_action_set(btn_poweroff, poweroff);
  button_action_set(btn_start,    temperature_daemon_start);
  button_action_set(btn_stop,     temperature_daemon_stop);

  // Enable buttons
  button_enable(btn_poweroff, BTN_VOLT_HIGH);
  button_enable(btn_start,    BTN_VOLT_HIGH);
  button_enable(btn_stop,     BTN_VOLT_HIGH);
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


int main(void) {
  power_setup();

  while (1) { // Main application loop
    // Enable and light LEDs
    led_enable(POWER_ON_LED);
    led_enable(POWER_ACT_LED);
    led_on(POWER_ON_LED);
    led_on(POWER_ACT_LED);

    config_fetch(); // Fetch avrtmon configuration

    temperature_setup();
    buttons_setup();

    command_init();
    communication_init();

    perform_setup = 0;
    sei();

    // Power-on application loop
    while (!perform_setup) {
      act_perf = 0;
      act_perf |= communication_handler();      // Check for incoming packets
      act_perf |= temperature_daemon_handler(); // Check for new temperatures
      act_perf |= button_handler();             // Check for user interaction

      // Enter (interruptable) sleep mode if no action request was performed
      sleep_on(SLEEP_MODE_IDLE, !act_perf);
    }
  }
}
