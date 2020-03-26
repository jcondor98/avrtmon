// avrtmon
// Daemon for registering temperatures - Source file
// Paolo Lucchesi - Thu 26 Mar 2020 04:05:12 PM CET
#include <avr/interrupt.h>

#include "temperature_daemon.h"
#include "temperature.h"
#include "lmsensor.h"

#define OCR_ONE_MSEC 15.625

// Set/Clear timer interrupt flag
static inline void td_sei(void) { TIMSK1 |=  (1 << OCIE1A); }
static inline void td_cli(void) { TIMSK1 &= ~(1 << OCIE1A); }


// Timer variables
static volatile uint16_t timer_counter;
static volatile uint8_t timer_ongoing;
static uint16_t timer_resolution;
static uint16_t timer_interval;


// Timer ISR -- Timer 1 is used
ISR(TIMER1_COMPA_vect) {
  if (++timer_counter != timer_interval) return;
  timer_counter = 0;
  lm_start_conv();
}


// Initialize the daemon (inlcuding related timer)
void temperature_daemon_init(uint16_t tim_resolution, uint16_t tim_interval) {
  // Set the prescaler to 1024
  TCCR1A = 0;
  TCCR1B = (1 << WGM52) | (1 << CS50) | (1 << CS52);

  // Set the OCR value to 'tim_resolution' milliseconds
  OCR1A = (uint16_t)(OCR_ONE_MSEC * tim_resolution);
  timer_resolution = tim_resolution;

  timer_interval = tim_interval;
}

// Start/Stop the daemon -- Button friendly (but 'pressed' will be ignored)
void temperature_daemon_start(uint8_t pressed) {
  if (timer_ongoing || temperature_db_new() != 0) return;
  timer_counter = 0;
  td_sei();
  timer_ongoing = 1;
}

void temperature_daemon_stop(uint8_t pressed) {
  td_cli();
  timer_ongoing = 0;
}

// Is the daemon running?
uint8_t temperature_daemon_ongoing(void) { return timer_ongoing; }

// Get timer resolution/interval
// TODO: Setters?
uint16_t temperature_daemon_get_resolution(void) { return timer_resolution; }
uint16_t temperature_daemon_get_interval(void)   { return timer_interval; }

// Handle daemon "notifications", must be run periodically
// In practice, register new temperatures if there is any
void temperature_daemon_handler(void) {
  // If no new temperatures are available, return immediately
  if (!lm_available()) return;

  // Stop the timer if there is no space left in the NVM
  if (temperature_register(lm_getresult()) != 0)
    temperature_daemon_stop(1);
}
