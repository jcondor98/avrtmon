// avrtmon
// Daemon for registering temperatures - Head file
// Paolo Lucchesi - Thu 26 Mar 2020 04:05:12 PM CET
#ifndef __TEMPERATURE_DAEMON_H
#define __TEMPERATURE_DAEMON_H
#include <stdint.h>


// Initialize the daemon (inlcuding related timer)
void temperature_daemon_init(uint16_t tim_resolution, uint16_t tim_interval);

// Start/Stop the daemon -- Button friendly (but 'pressed' will be ignored)
void temperature_daemon_start(uint8_t pressed);
void temperature_daemon_stop(uint8_t pressed);

// Is the daemon running? (0=stopped, 1=running)
uint8_t temperature_daemon_ongoing(void);

// Get registering timer interval and resolution
uint16_t temperature_daemon_get_resolution(void);
uint16_t temperature_daemon_get_interval(void);

// Handle daemon "notifications", must be run periodically
void temperature_daemon_handler(void);

#endif    // __TEMPERATURE_DAEMON_H
