// avrtmon
// Buttons Handler - Source file
// Paolo Lucchesi - Tue 29 Oct 2019 11:57:18 AM CET
#include <avr/interrupt.h>
#include "lmsensor_timer.h"
#include "buttons.h"


// Buttons used in the module
button_t buttons[BUTTON_COUNT] = { 0 };

// Bitmasks for PINs
static const uint8_t pin_mask[] = {
  1 << D53, 1 << D52, 1 << D51, 1 << D50
};


// External Interrupt ISR
ISR(PCINT0_vect) {
  uint8_t portb_in = PINB;

  // PORTB pins: Increment if pressed, loop unrolled
  buttons[0].pressed += (portb_in >> 0) & 0xFE;
  buttons[1].pressed += (portb_in >> 1) & 0xFE;
  buttons[2].pressed += (portb_in >> 2) & 0xFE;
  buttons[3].pressed += (portb_in >> 3) & 0xFE;
}


// Initialize the buttons handler (just enable PCINT with interrupts on no pins)
void buttons_init(void) {
  PCICR = PCIE0;
  PCMSK0 = 0;
}


// The buttons handler itself
// Stops the lmsensor timer and registering if 'status == STATUS_STOPPED' and
// it is running, and vice-versa
void buttons_handler(void) {
  for (uint8_t id=0; id < BUTTON_COUNT; ++id) {
    if (!buttons[id].enabled || !buttons[id].action || !buttons[id].pressed)
      continue;
    buttons[id].action(buttons[id].pressed);
    buttons[id].pressed = 0;
  }
}


// Set a callback for a button (NULL is non-sensical but accepted)
// Returns 0 on success, 1 if the button does not exist
uint8_t button_action_set(button_id_t id, button_callback_t action) {
  if (id >= BUTTON_COUNT) return 1;
  if (buttons[id].enabled) button_disable(id);
  buttons[id].pressed = 0;
  buttons[id].action = action;
  return 0;
}


// Enable/disable interrupt for buttons
// Returns 0 on success, 1 if the button does not exist
uint8_t button_enable(button_id_t id) {
  if (id >= BUTTON_COUNT)  return 1;
  if (buttons[id].enabled) return 0;
  buttons[id].enabled = 1;
  PCMSK0 |= pin_mask[id];
  return 0;
}

uint8_t button_disable(button_id_t id) {
  if (id >= BUTTON_COUNT)   return 1;
  if (!buttons[id].enabled) return 0;
  buttons[id].enabled = 0;
  PCMSK0 &= ~pin_mask[id];
  return 0;
}
