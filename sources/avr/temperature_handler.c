// avrtmon
// Temperature handler
// Paolo Lucchesi - Mon 28 Oct 2019 01:31:23 AM CET
#include "temperature.h"
#include "lmsensor_timer.h"
#include "lmsensor.h"


void temperature_handler(void) {
  // If no new temperatures are available, return immediately
  if (!lm_available()) return;

  // If the DB is full stop the timer, else try to register a new temperature
  if (temperature_count() == temperature_capacity())
    lm_timer_stop();
  else
    temperature_register(lm_getresult());
}
