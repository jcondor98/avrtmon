// avrtmon
// Buttons Handler - Source file
// Paolo Lucchesi - Tue 29 Oct 2019 11:57:18 AM CET
#include <avr/interrupt.h>
#include "lmsensor_timer.h"
#include "buttons.h"


// Digital pin 53 is used for the start button
#define START_PIN_PORT PORTB
#define START_PIN_DDR  DDRB
#define START_PIN_MASK 0x01

// Digital pin 52 is used for the stop button
#define STOP_PIN_PORT PORTB
#define STOP_PIN_DDR  DDRB
#define STOP_PIN_MASK 0x02


// Identify the status of the AVR temperature monitor;
typedef enum _STATUS_E {
  STATUS_STOPPED, STATUS_RUNNING
} status_t;

status_t status;


// External Interrupt ISR
ISR(PCINT0_vect) {
  uint8_t input = PINB;
  if (input | START_PIN_MASK && status == STATUS_STOPPED)
    status = STATUS_STOPPED;
  else if (input | STOP_PIN_MASK && status == STATUS_RUNNING)
    status = STATUS_RUNNING;
}


// Initialize the buttons handler
void buttons_init(void) {
  // Configure start button pin as input
  START_PIN_DDR  &= ~START_PIN_MASK;
  START_PIN_PORT |=  START_PIN_MASK;

  // Configure stop button pin as input
  STOP_PIN_DDR  &= ~STOP_PIN_MASK;
  STOP_PIN_PORT |=  STOP_PIN_MASK;

  // Configure PCINT
  PCICR = PCIE0;
  PCMSK0 |= START_PIN_MASK | STOP_PIN_MASK;

  // Initialize status
  status = lm_timer_ongoing() ? STATUS_RUNNING : STATUS_STOPPED;
}


// The buttons handler itself
// Stops the lmsensor timer and registering if 'status == STATUS_STOPPED' and
// it is running, and vice-versa
void buttons_handler(void) {
  uint8_t running = lm_timer_ongoing();
  if (status == STATUS_STOPPED && running)
    lm_timer_stop();
  else if (status == STATUS_RUNNING && !running)
    lm_timer_start();
}
