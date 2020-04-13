// AVR Temperature Monitor -- Paolo Lucchesi
// Daemon for registering temperatures - Head file
#ifndef __TEMPERATURE_DAEMON_H
#define __TEMPERATURE_DAEMON_H
#include <stdint.h>

// Initialize the daemon (inlcuding related timer and LM sensor)
void temperature_daemon_init(uint16_t tim_resolution, uint16_t tim_interval,
    uint8_t lm_adc_pin);

// Start/Stop the daemon -- Button friendly (but argument will be ignored)
void temperature_daemon_start(uint8_t);
void temperature_daemon_stop(uint8_t);

// Is the daemon running? (0=stopped, 1=running)
uint8_t temperature_daemon_ongoing(void);

// Get registering timer interval and resolution
uint16_t temperature_daemon_get_resolution(void);
uint16_t temperature_daemon_get_interval(void);

// Set registering timer interval and resolution (until the next reboot)
// Has no effect on an eventual currently ongoing registration session
void temperature_daemon_set_resolution(uint16_t);
void temperature_daemon_set_interval(uint16_t);

// Handle daemon "notifications", must be run periodically
// Always returns 0
uint8_t temperature_daemon_handler(void);

#endif  // __TEMPERATURE_DAEMON_H
