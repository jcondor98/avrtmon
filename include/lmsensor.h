// avrtmon
// LM Sensor layer - Head file
// Paolo Lucchesi - Sun 18 Aug 2019 05:28:53 PM CEST
#ifndef __LM_SENSON_LAYER_H
#define __LM_SENSON_LAYER_H
#include <stdint.h>

// Initialize ADC and other required stuff
void lm_init(void);

// Start a conversion (and return 0)
// If the ADC is busy, do not start the conversion and return 1
uint8_t lm_start_conv(void);

// Return 1 if there is an ongoing conversion, 0 otherwise
uint8_t lm_busy(void);

// Returns 1 if a registered temperature is available, 0 otherwise
uint8_t lm_available(void);

// Get the last LM registered temperature
uint16_t lm_getresult(void);

#endif    // __LM_SENSON_LAYER_H
