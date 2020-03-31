// avrtmon
// Buttons Handler - Source file
// Paolo Lucchesi - Tue 29 Oct 2019 11:57:18 AM CET
#include <avr/interrupt.h>
#include "buttons.h"

#define OCR_ONE_MSEC 15.625
#define DEBOUNCE_TIME 500


// Buttons used in the module
button_t buttons[BUTTON_COUNT] = { 0 };

// Bitmasks for PINs
static const uint8_t pin_mask[] = {
  1 << D53, 1 << D52, 1 << D51, 1 << D50
};

// Toggle Interrupt enabled flags
static inline void eint_sei(void) { PCICR |=  (1 << PCIE0); }
static inline void eint_cli(void) { PCICR &= ~(1 << PCIE0); }
static inline void tim4_sei(void) { TIMSK4 |=  (1 << OCIE4A); }
static inline void tim4_cli(void) { TIMSK4 &= ~(1 << OCIE4A); }


// External Interrupt ISR
ISR(PCINT0_vect) {
  uint8_t portb_in = ~PINB; // Closed button is a 0 in PIN* for our schematics

  // PORTB pins: Increment if pressed, loop unrolled
  buttons[D53].pressed += (portb_in >> D53) & 0x01;
  buttons[D52].pressed += (portb_in >> D52) & 0x01;
  buttons[D51].pressed += (portb_in >> D51) & 0x01;
  buttons[D50].pressed += (portb_in >> D50) & 0x01;
}

/*
// Debouncer ISR -- Timer 4 is used
ISR(TIMER4_COMPA_vect) {
  if (!(PINB & debouncing_btn_mask)) return;
  debouncing_btn_mask = 0;
  debouncing = 0;
  eint_sei();
  tim4_cli();
}
*/


// Initialize the buttons handler (just enable PCINT with interrupts on no pins)
void button_init(void) {
  // Enable external interrupts
  eint_sei();
  PCMSK0 = 0;

  // Initialize debouncer timer
  // Set prescaler to 1024
  TCCR4A = 0;
  TCCR4B = (1 << WGM52) | (1 << CS50) | (1 << CS52);

  OCR4A = (uint16_t)(OCR_ONE_MSEC * DEBOUNCE_TIME); // Set debounce time
}


// The buttons handler itself
void button_handler(void) {
  for (uint8_t btn=0; btn < BUTTON_COUNT; ++btn) {
    if (!buttons[btn].enabled || !buttons[btn].action || !buttons[btn].pressed)
      continue;
    buttons[btn].action(buttons[btn].pressed);
    buttons[btn].pressed = 0;
  }
}


// Set a callback for a button (NULL is non-sensical but accepted)
// Returns 0 on success, 1 if the button does not exist
uint8_t button_action_set(button_pin_t id, button_callback_f action) {
  if (id >= BUTTON_COUNT) return 1;
  if (buttons[id].enabled) button_disable(id);
  buttons[id].pressed = 0;
  buttons[id].action = action;
  return 0;
}


// Enable/disable interrupt for buttons
// Returns 0 on success, 1 if the button does not exist
uint8_t button_enable(button_pin_t id) {
  if (id >= BUTTON_COUNT)  return 1;
  if (buttons[id].enabled) return 0;
  buttons[id].enabled = 1;
  DDRB   &= ~pin_mask[id];
  PORTB  |=  pin_mask[id];
  PCMSK0 |=  pin_mask[id];
  return 0;
}

uint8_t button_disable(button_pin_t id) {
  if (id >= BUTTON_COUNT)   return 1;
  if (!buttons[id].enabled) return 0;
  buttons[id].enabled = 0;
  PCMSK0 &= ~pin_mask[id];
  PORTB  &= ~pin_mask[id];
  DDRB   |=  pin_mask[id];
  return 0;
}
