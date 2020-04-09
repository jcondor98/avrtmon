// AVR Temperature Monitor -- Paolo Lucchesi
// LM Sensor layer - Head file
#ifndef __LMSENSOR_MODULE_H
#define __LMSENSOR_MODULE_H
#include <stdint.h>

// Configurable ADC analog pin used for the LM35
typedef enum ADC_PIN_E {
  A0 = 0, A1, A2, A3, A4, A5, A6, A7
} adc_pin_t;


// Initialize ADC and other required stuff
void lm_init(uint8_t adc_pin);

// Start a conversion and enter in ADC Noise Reduction sleep mode until the
// conversion completes
// Returns the registered temperature, or 0 on failure
uint8_t lm_convert(void);

#endif  // __LMSENSOR_MODULE_H
