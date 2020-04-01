// avrtmon
// LM Sensor layer - Head file
// Paolo Lucchesi - Sun 18 Aug 2019 05:28:53 PM CEST
#ifndef __LM_SENSOR_LAYER_H
#define __LM_SENSOR_LAYER_H
#include <stdint.h>

// Configurable ADC analog pin used for the LM35
typedef enum ADC_PIN_E {
  A0 = 0, A1, A2, A3, A4, A5, A6, A7
} adc_pin_t;


// Initialize ADC and other required stuff
void lm_init(uint8_t adc_pin);

// Start a conversion (and return 0)
// If the ADC is busy, do not start the conversion and return 1
uint8_t lm_start_conv(void);

// Return 1 if there is an ongoing conversion, 0 otherwise
uint8_t lm_ongoing(void);

// Returns 1 if a registered temperature is available, 0 otherwise
uint8_t lm_available(void);

// Get the last registered temperature (as a raw tension converted by the ADC)
uint16_t lm_getresult(void);

#endif    // __LM_SENSOR_LAYER_H
