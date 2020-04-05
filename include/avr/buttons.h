// avrtmon
// Buttons Handler - Source file
// Paolo Lucchesi - Tue 29 Oct 2019 01:19:42 PM CET
#ifndef __BUTTONS_HANDLER_H
#define __BUTTONS_HANDLER_H
#include <stdint.h>

// Identifiers for different pins (used for buttons)
// Pins are PCINT[0,3], i.e. Digital[53,50] (but might be scaled in future)
typedef enum BUTTON_PIN_E {
  D53 = 0, D52, D51, D50, D21
} button_pin_t;
#define BUTTON_COUNT 5

// Voltage which pass through a button
// Where BTN_VOLT_AUTO is supported and passed, the buttons module will try to
// determine the voltage automatically, but it is known to be bugged in many
// situations
typedef enum BUTTON_VOLTAGE_E {
  BTN_VOLT_LOW = 0, BTN_VOLT_HIGH = 0x01, BTN_VOLT_AUTO
} button_voltage_t;

// Button status -- 'END' is used for shaft-encoder-like operations, e.g.:
//   status = (status + 1) % BTN_STAT_END
typedef enum BUTTON_STATUS_E {
  BTN_STAT_OPEN = 0, BTN_STAT_PRESSING, BTN_STAT_PRESSED, BTN_STAT_END
} button_status_t;

// Callback executed on button pression
typedef void (*button_callback_f)(uint8_t status);

// Button data type
typedef struct _button_s {
  button_callback_f action; // Action performed when a button is pressed
  volatile uint8_t history; // History of status changes, LSB is most recent
  volatile uint8_t status;  // Status of the button
  uint8_t enabled;          // Enable/Disable callback execution
} button_t;


// Initialize the buttons handler
// If 'debounce_tm' is 0, a DEFAULT_DEBOUNCE_TIME is used
void button_init(uint8_t debounce_tm);

// The buttons handler itself
// If some buttons have been pressed, the linked callbacks will be executed
// Returns 0 if no significant action was performed, non-zero otherwise
uint8_t button_handler(void);

// Set a callback for a button (NULL is non-sensical but accepted)
// Returns 0 on success, 1 if the button does not exist
// NOTE: The involved button will be always disabled after this call
uint8_t button_action_set(button_pin_t id, button_callback_f action);

// Enable/disable interrupt for buttons
// Returns 0 on success, 1 if the button does not exist
uint8_t button_enable(button_pin_t, button_voltage_t v_initial);
uint8_t button_disable(button_pin_t);

#endif    // __BUTTONS_HANDLER_H
