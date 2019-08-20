// avrtmon
// LM Sensor layer - Source file
// Paolo Lucchesi - Sun 18 Aug 2019 05:31:33 PM CEST
#include <avr/io.h>
#include <avr/interrupt.h>
#include "lmsensor.h"


volatile uint16_t adc_result;
volatile uint8_t  adc_available;
volatile uint8_t  adc_ongoing;


ISR(ADC_vect) {
  adc_available = 1;
  adc_ongoing = 0;
  adc_result = ADC;
}


// Initialize ADC and other required stuff
void lm_init(void) {
  // REFS1 -> Internal 2.56V Reference (no external AREF)
  // TODO: Take the analog pin (1-13, i.e. channel) as a parameter
  //   With no MUX* set, the taken analog pin is AD0 (i.e. ADC0 channel)
  ADMUX = (1 << REFS1);

  // ADEN  -> Enable ADC
  // ADIE  -> Enable Conversion Complete Interrupt
  // ADPS* -> Set prescaler to 128 (i.e. ADC Frequency = F_CPU / Psc. = 125kHz)
  ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

  // Disable digital I/O for analog pins
  DIDR0 = 0;
  DIDR2 = 0;
}


// Start a conversion (and return 0)
// If the ADC is busy, do not start the conversion and return 1
uint8_t lm_start_conv(void) {
  if (adc_ongoing)
    return 1;
  adc_ongoing = 1;
  ADCSRA |= (1 << ADSC);
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

// Get the last registered temperature (in Celsius)
// Note that side-effects expected by 'lm_getresult' will happen
float lm_getresult_celsius(void) {
  return ((float) lm_getresult()) / 10.0f;
}
