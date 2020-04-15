// AVR Temperature Monitor -- Paolo Lucchesi
// Daemon for registering temperatures - Source file
#include <avr/interrupt.h>
#include <util/delay.h>

#include "temperature_daemon.h"
#include "temperature.h"
#include "lmsensor.h"
#include "led.h"

#define MIN_REGISTRATION_INTERVAL 50
#define OCR_ONE_MSEC 15.625


// Timer variables
static volatile uint16_t timer_counter;
static volatile uint8_t timer_ongoing;
static volatile uint8_t timer_elapsed;
static uint16_t timer_resolution;
static uint16_t timer_interval;


// Start/Stop the daemon timer
static inline void timd_stop(void) { TIMSK1 &= ~(1 << OCIE1A); }
static inline void timd_start(void) {
  OCR1A = (uint16_t)(OCR_ONE_MSEC * timer_resolution);
  TCNT1 = 0;
  TIMSK1 |=  (1 << OCIE1A);
}

// Timer ISR -- Timer 1 is used
ISR(TIMER1_COMPA_vect) {
  if (++timer_counter != timer_interval) return;
  timer_counter = 0;
  timer_elapsed = 1;
}


// Initialize the daemon (inlcuding related timer and LM sensor)
void temperature_daemon_init(uint16_t tim_resolution, uint16_t tim_interval,
    uint8_t lm_adc_pin) {
  led_enable(TEMPERATURE_REGISTERING_LED);

  // Set the prescaler to 1024
  TCCR1A = 0;
  TCCR1B = (1 << WGM52) | (1 << CS50) | (1 << CS52);

  timer_resolution = tim_resolution;
  timer_interval = tim_interval;

  // Initialize LM sensor module
  lm_init(lm_adc_pin);
}


// Start/Stop the daemon -- Button friendly (but 'pressed' will be ignored)
void temperature_daemon_start(uint8_t pressed) {
  if (timer_ongoing ||
      temperature_db_new(timer_resolution, timer_interval) != 0 ||
      timer_resolution * timer_interval < MIN_REGISTRATION_INTERVAL)
    return;

  led_on(TEMPERATURE_REGISTERING_LED);
  timer_counter = 0;
  timer_ongoing = 1;
  timd_start();
}


void temperature_daemon_stop(uint8_t pressed) {
  timd_stop();
  timer_ongoing = 0;
  led_off(TEMPERATURE_REGISTERING_LED);
}

// Is the daemon running?
uint8_t temperature_daemon_ongoing(void) { return timer_ongoing; }

// Get timer resolution/interval
uint16_t temperature_daemon_get_resolution(void) { return timer_resolution; }
uint16_t temperature_daemon_get_interval(void)   { return timer_interval; }

// Set registering timer interval and resolution (until the next reboot)
// Has no effect on an eventual currently ongoing registration session
void temperature_daemon_set_resolution(uint16_t tres) {
  if (tres) timer_resolution = tres;
}

void temperature_daemon_set_interval(uint16_t tint) {
  if (tint) timer_interval = tint;
}


// Handle daemon "notifications", must be run periodically
// In practice, register new temperatures if there is any
uint8_t temperature_daemon_handler(void) {
  if (!timer_elapsed) return 0;
  timer_elapsed = 0;

  if (temperature_register(lm_convert()) != 0)
    temperature_daemon_stop(1);
  return 0; // Always returns 0
}
