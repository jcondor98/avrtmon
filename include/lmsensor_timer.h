// avrtmon
// ADC/lmsensor Timer module - Head file
// Paolo Lucchesi - Mon 28 Oct 2019 01:04:26 AM CET
#ifndef __LM_SENSOR_TIMER_H
#define __LM_SENSOR_TIMER_H

// Initialize the timer
void lm_timer_init(void);

// Start the timer
void lm_timer_start(void);

// Stop the timer
void lm_timer_stop(void);

// Return 1 if the timer is running, 0 if it is stopped
uint8_t lm_timer_ongoing(void);

#endif    // __LM_SENSOR_TIMER_H
