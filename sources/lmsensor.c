// avrtmon
// LM Sensor layer - Source file
// Paolo Lucchesi - Sun 18 Aug 2019 05:31:33 PM CEST
#include <avr/io.h>
#include "lmsensor.h"


volatile uint16_t adc_result;
volatile uint8_t  adc_available;
volatile uint8_t  adc_busy;


ISR(ADC_vect) {
  adc_available = 1;
  adc_busy = 0;
  adc_result = ADC;
}


// Initialize ADC and other required stuff
void lm_init(void) {
  // REFS0 -> AVCC with capacitor connected to AREF pin
  // TODO: Take the analog pin (1-13, i.e. channel) as a parameter
  //   With no MUX* set, the taken analog pin is A0
  ADMUX = (1 << REFS0);

  // ADEN  -> Enable ADC
  // ADPS* -> Set prescaler to 128 (i.e. ADC Frequency = F_CPU / Psc. = 125kHz)
  // ADIE  -> Enable Conversion Complete Interrupt
  // [NOT ENABLED] ADATE -> Enable Auto Triggering Conversions
  //   Every conversion will be started manually (e.g. by a timer ISR)
  ADCSRA = (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

  // Disable digital I/O for analog pins
  DIDR0 = 0;
  DIDR2 = 0;
}


// Start a conversion (and return 0)
// If the ADC is busy, do not start the conversion and return 1
uint8_t lm_start_conv(void) {
  if (adc_busy)
    return 1;
  adc_busy = 1;
  ADCSRA |= (1 << ADSC);
  return 0;
}

// Return 1 if there is an ongoing conversion, 0 otherwise
uint8_t lm_busy(void) { return adc_busy; }

// Returns 1 if a registered temperature is available, 0 otherwise
uint8_t lm_available(void) { return adc_available; }

// Get the LM temperature from a raw ADC conversion result
// TODO: Process the raw ADC result
uint16_t lm_getresult(void) {
  adc_available = 0; // Last result will be returned anyway
  return adc_result;
}


