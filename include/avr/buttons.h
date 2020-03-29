// avrtmon
// Buttons Handler - Source file
// Paolo Lucchesi - Tue 29 Oct 2019 01:19:42 PM CET
#ifndef __BUTTONS_HANDLER_H
#define __BUTTONS_HANDLER_H
#include <stdint.h>

// Identifiers for different pins (used for buttons)
// Pins are PCINT[0,3], i.e. Digital[53,50] (but might be scaled in future)
typedef enum BUTTON_PIN_E {
  D53 = 1 << 0, D52 = 1 << 1, D51 = 1 << 2, D50 = 1 << 3
} button_pin_t;
#define BUTTON_COUNT 4

// Callback executed on button pression
typedef void (*button_callback_f)(uint8_t times_pressed);

// Button data type
typedef struct _button_s {
  button_callback_f action; // Action performed when a button is pressed
  volatile uint8_t pressed; // How many times a button have been pressed?
  uint8_t enabled;          // Enable/Disable callback execution
} button_t;


// Initialize the buttons handler
void button_init(void);

// The buttons handler itself
// If one or more buttons have been pressed, the linked callbacks will be executed
void button_handler(void);

// Set a callback for a button
// Returns 0 on success, 1 if the button does not exist
uint8_t button_action_set(button_pin_t id, button_callback_f action);

// Enable/disable interrupt for buttons
// Returns 0 on success, 1 if the button does not exist
uint8_t button_enable(button_pin_t);
uint8_t button_disable(button_pin_t);

#endif    // __BUTTONS_HANDLER_H
