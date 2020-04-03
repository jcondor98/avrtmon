// avrtmon
// LM Sensor layer - Source file
// Paolo Lucchesi - Sun 18 Aug 2019 05:31:33 PM CEST
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "lmsensor.h"

#define POWER_ACT_LED D24

volatile uint16_t adc_result;
volatile uint8_t  adc_available;
volatile uint8_t  adc_ongoing;


// ADC conversion complete ISR
ISR(ADC_vect) {
  adc_available = 1;
  adc_ongoing = 0;
  adc_result = ADC;
  lm_cli(); // Do not raise interrupts further
}


// Initialize ADC and other required stuff
void lm_init(uint8_t adc_pin) {
  // REFS1 -> Internal 2.56V Reference (no external AREF)
  // Select ADC pin configured to handle the LM35 sensor
  ADMUX = (1 << REFS1) | (adc_pin & 0x1F);

  // ADEN  -> Enable ADC
  // ADIE  -> Enable Conversion Complete Interrupt
  // ADPS* -> Set prescaler to 128 (i.e. ADC Frequency = F_CPU / Psc. = 125kHz)
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

  // Perform and discard first conversion
  ADCSRA |= (1 << ADSC);
  while(ADCSRA & (1 << ADSC)) ;
}


// Start a conversion (and return 0)
// If the ADC is busy, do not start the conversion and return 1
uint8_t lm_convert(void) {
  // Setup before the conversion (do not go to sleep if global interrupts are
  // disabled, you would sleep - almost - forever)
  if (adc_ongoing || !(SREG & (1 << 7))) return 1;
  adc_ongoing = 1;
  lm_sei();

  // Enter in ADC Noise Reduction sleep mode
  // Assume that global interrupts are enabled (libraries should never 'sei()')
  SMCR &= ~(1 << SM2 | 1 << SM1);
  SMCR |= 1 << SM0;
  sleep_enable();
  sleep_cpu();
  sleep_disable();

  return 0;
}

// Return 1 if there is an ongoing conversion, 0 otherwise
uint8_t lm_ongoing(void) { return adc_ongoing; }

// Returns 1 if a registered temperature is available, 0 otherwise
uint8_t lm_available(void) { return adc_available; }

// Get the LM temperature from a raw ADC conversion result
uint16_t lm_getresult(void) {
  adc_available = 0; // Last result will be returned anyway
  return adc_result;
}
