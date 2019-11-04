// avrtmon
// Buttons Handler - Source file
// Paolo Lucchesi - Tue 29 Oct 2019 01:19:42 PM CET
#ifndef __BUTTONS_HANDLER_H
#define __BUTTONS_HANDLER_H
#include <stdint.h>

// Initialize the buttons handler
void buttons_init(void);

// The buttons handler itself
// Stops the lmsensor timer and registering if the stop button is pressed and
// the temperature monitor is running, and vice-versa
void buttons_handler(void);


#endif    // __BUTTONS_HANDLER_H

