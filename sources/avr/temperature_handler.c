// avrtmon
// Temperature handler
// Paolo Lucchesi - Mon 28 Oct 2019 01:31:23 AM CET
#include "temperature.h"
#include "lmsensor_timer.h"
#include "lmsensor.h"


void temperature_handler(void) {
  // If no new temperatures are available, return immediately
  if (!lm_available()) return;

  // Stop the timer if there is no space left in the NVM
  if (temperature_register(lm_getresult()) != 0)
    lm_timer_stop();
}
