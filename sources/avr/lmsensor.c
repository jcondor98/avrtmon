// AVR Temperature Monitor -- Paolo Lucchesi
// LM Sensor layer - Source file
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "lmsensor.h"

#define POWER_ACT_LED D24

// Set/Clear ADC interrupt flag
#define lm_sei() do { ADCSRA |=  (1 << ADIE); } while (0)
#define lm_cli() do { ADCSRA &= ~(1 << ADIE); } while (0)

volatile uint16_t adc_result;
volatile uint8_t  adc_ongoing;


// ADC conversion complete ISR
ISR(ADC_vect) {
  adc_ongoing = 0;
  adc_result = ADC;
  lm_cli(); // Do not raise interrupts further
}


// Initialize ADC and other required stuff
void lm_init(uint8_t adc_pin) {
  adc_ongoing = 0;

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


// Start a conversion and enter in ADC Noise Reduction sleep mode until the
// conversion completes
// Returns the registered temperature, or 0 on failure
uint8_t lm_convert(void) {
  // Do not go to sleep if global interrupts are disabled
  if (! (SREG & (1 << 7))) return 0;
  lm_sei();

  // Enter in ADC Noise Reduction sleep mode
  // Assume that global interrupts are enabled (libraries should never 'sei()')
  SMCR &= ~(1 << SM2 | 1 << SM1);
  SMCR |= 1 << SM0;
  sleep_enable();
  sleep_cpu();
  sleep_disable();

  // Busy wait if the conversion was interrupted
  while (adc_ongoing) ;

  return adc_result;
}
