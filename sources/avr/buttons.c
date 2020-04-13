// AVR Temperature Monitor -- Paolo Lucchesi
// Buttons Handler - Source file
#include <avr/interrupt.h>
#include "buttons.h"

#define OCR_ONE_MSEC 15.625

// Buttons used in the module
static button_t buttons[BUTTON_COUNT] = { 0 };

// Debounce variables (flag and time in milliseconds)
static uint8_t debounce_time, debouncing;


// Get external interrupt flags
static inline uint8_t eint_flg(void) { return PCICR & (1 << PCIE0 ); }
static inline uint8_t int0_flg(void) { return EIMSK & (1 << INT0  ); }

// Set/Clear external interrupt and debouncer timer flags
static inline void eint_sei(void) { PCIFR |= 1 << PCIF0; PCICR |= 1 << PCIE0; }
static inline void int0_sei(void) { EIFR  |= 1 << INTF0; EIMSK |= 1 << INT0;  }
static inline void tim4_sei(void) { TIMSK4 |=  (1 << OCIE4A); }
static inline void eint_cli(void) { PCICR  &= ~(1 << PCIE0 ); }
static inline void int0_cli(void) { EIMSK  &= ~(1 << INT0  ); }
static inline void tim4_cli(void) { TIMSK4 &= ~(1 << OCIE4A); }

// Reset button history giving current input state
static inline void _btn_stat_reset(button_t *btn, uint8_t input) {
  btn->status = BTN_STAT_OPEN;
  btn->history = input ? 0xFF : 0x00;
}

// Return the updated button status after a pin change interrupt
static inline uint8_t _btn_update(button_t *btn, uint8_t input) {
  uint8_t history = (btn->history << 1) | input;
  uint8_t changed = (history & 0x01) ^ ((history & 0x02) >> 1);
  uint8_t status = btn->status;
  if (changed && status < BTN_STAT_PRESSED) {
      btn->status = ++status;
      btn->history = history;
  }
  return status;
}


// Debouncer ISR -- Timer 4 is used
ISR(TIMER4_COMPA_vect) {
  debouncing = 0;

  // Debounce [D53,D50] (i.e. PCINT0[0,3])
  if (!eint_flg()) {
    uint8_t pressing = 0;
    for (uint8_t btn=D53, pinb=PINB; btn <= D50; ++btn)
      if (buttons[btn].enabled &&
          _btn_update(&buttons[btn], (pinb >> btn) & 0x01) == BTN_STAT_PRESSING)
        ++pressing;
    if (!pressing) eint_sei();
    debouncing |= pressing;
  }

  // Debounce D21 (i.e. INT0)
  if (!int0_flg() && buttons[D21].enabled) {
    uint8_t status = _btn_update(&buttons[D21], PIND & 0x01);
    if (status == BTN_STAT_PRESSING)
      ++debouncing;
    else int0_sei();
  }

  if (!debouncing) tim4_cli(); // All debounced
}


// PCINT0 External Interrupt ISR (start debounce timer)
ISR(PCINT0_vect) {
  uint8_t pinb = PINB;
  for (uint8_t btn=D53; btn <= D50; ++btn)
    if (buttons[btn].enabled)
      _btn_update(&buttons[btn], (pinb >> btn) & 0x01);
  eint_cli();
  tim4_sei();
}

// INT0 ISR (start debounce timer)
ISR(INT0_vect) {
  _btn_update(&buttons[D21], PIND & 0x01);
  int0_cli();
  tim4_sei();
}


// Initialize the buttons handler (just enable PCINT with interrupts on no pins)
// If 'debounce_tm' is 0, a DEFAULT_DEBOUNCE_TIME is used
void button_init(uint8_t debounce_tm) {
  // Initialize debouncer timer
  // Set prescaler to 1024
  TCCR4A = 0;
  TCCR4B = (1 << WGM52) | (1 << CS50) | (1 << CS52);

  debounce_time = debounce_tm ? debounce_tm : DEFAULT_DEBOUNCE_TIME;
  OCR4A = (uint16_t)(OCR_ONE_MSEC * debounce_time); // Set debounce time

  PCMSK0 = 0;
}


// The buttons handler itself
// Returns 0 if no significant action was performed, non-zero otherwise
uint8_t button_handler(void) {
  uint8_t ret = 0;

  for (uint8_t btn=0; btn < BUTTON_COUNT; ++btn) {
    if (buttons[btn].enabled && buttons[btn].action) {
      if (buttons[btn].status == BTN_STAT_PRESSING)
        ret = 1;
      else if (buttons[btn].status == BTN_STAT_PRESSED) {
        buttons[btn].action(buttons[btn].status);
        buttons[btn].status = BTN_STAT_OPEN;
      }
    }
  }

  return ret;
}


// Set a callback for a button (NULL is non-sensical but accepted)
// Returns 0 on success, 1 if the button does not exist
// NOTE: The involved button will be always disabled after this call
uint8_t button_action_set(button_pin_t id, button_callback_f action) {
  if (id >= BUTTON_COUNT) return 1;
  if (buttons[id].enabled) button_disable(id);
  buttons[id].action = action;
  return 0;
}


// Enable interrupt for buttons
// Returns 0 on success, 1 if the button does not exist
uint8_t button_enable(button_pin_t btn, button_voltage_t v_initial) {
  if (btn >= BUTTON_COUNT)  return 1;
  if (buttons[btn].enabled) return 0;
  register uint8_t mask; // Used to store computed pin mask

  switch (btn) {

    case D53: case D52: case D51: case D50: // PCINT0
      mask = 1 << btn;
      DDRB   &= ~mask;
      PORTB  |=  mask;
      if (v_initial == BTN_VOLT_AUTO)
        v_initial = PINB >> btn & 0x01;
      _btn_stat_reset(&buttons[btn], v_initial);
      buttons[btn].enabled = 1;
      PCMSK0 |=  mask;
      if (!debouncing) eint_sei();
      break;

    case D21: // INT0
      DDRD  &= ~0x01;
      PORTD |=  0x01;
      EICRA |=  (1 << ISC00);
      EICRA &= ~(1 << ISC01); // Interrupt on any edge
      if (v_initial == BTN_VOLT_AUTO)
        v_initial = PINB >> btn & 0x01;
      _btn_stat_reset(&buttons[D21], v_initial);
      buttons[btn].enabled = 1;
      int0_sei();
      break;

    default: return 1;
  }

  return 0;
}


// Disable interrupt for buttons
// Returns 0 on success, 1 if the button does not exist
uint8_t button_disable(button_pin_t btn) {
  if (btn >= BUTTON_COUNT)   return 1;
  if (!buttons[btn].enabled) return 0;
  register uint8_t mask; // Used to store computed pin mask

  switch (btn) {

    case D53: case D52: case D51: case D50: // PCINT0
      buttons[btn].enabled = 0;
      mask = 1 << btn;
      PCMSK0 &= ~mask;
      PORTB  &= ~mask;
      DDRB   |=  mask;
      if (! (buttons[D53].enabled || buttons[D52].enabled ||
             buttons[D51].enabled || buttons[D50].enabled))
          eint_cli();
      break;

    case D21: // INT0
      buttons[D21].enabled = 0;
      int0_cli();
      break;

    default: return 1;
  }

  return 0;
}
