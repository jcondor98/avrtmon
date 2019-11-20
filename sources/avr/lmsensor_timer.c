// avrtmon
// ADC/lmsensor Timer module - Source file
// Paolo Lucchesi - Mon 28 Oct 2019 12:23:27 AM CET
#include <avr/interrupt.h>
#include "lmsensor.h"
#include "config.h"


// Timer resolution in milliseconds
#define TIMER_RES 1000

// Use an external, wider counter for the timer
static volatile uint16_t counter;
static uint16_t interval;
static uint8_t ongoing;


// Timer ISR -- Timer 1 is used
ISR(TIMER1_COMPA_vect) {
  if (++counter == interval) {
    lm_start_conv();
    counter = 0;
  }
}


// Initialize the timer
void lm_timer_init(void) {
  config_get(CFG_LM_INTERVAL, &interval);

  // Set the prescaler to 1024
  TCCR1A = 0;
  TCCR1B = (1 << WGM52) | (1 << CS50) | (1 << CS52);

  // Set the OCR value
  OCR1A = (uint16_t)(15.625 * TIMER_RES);
}


// Start the timer
void lm_timer_start(void) {
  if (!ongoing) {
    cli();
    counter = 0;
    TIMSK1 |= (1 << OCIE1A);
    sei();
    ongoing = 1;
  }
}

// Stop the timer
void lm_timer_stop(void) {
  cli();
  TIMSK1 &= ~(1 << OCIE1A);
  sei();
  ongoing = 0;
}

// Getter for 'ongoing'
uint8_t lm_timer_ongoing(void) { return ongoing; }
