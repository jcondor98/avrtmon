// AVR Temperature Monitor -- Paolo Lucchesi
// Sleep utility
// NOTE: This interface permanently alters the global interrupt flag
#ifndef __SLEEP_UTIL_INTERFACE_H
#define __SLEEP_UTIL_INTERFACE_H
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "led.h"

#define sleep(mode) do {\
  uint8_t __sreg_bak = SREG;\
  set_sleep_mode(mode);\
  led_off(POWER_ACT_LED);\
  sleep_enable();\
  sei();\
  sleep_cpu();\
  sleep_disable();\
  led_on(POWER_ACT_LED);\
  SREG = __sreg_bak;\
} while (0);

#define sleep_on(mode,expr) do {\
  uint8_t __sreg_bak = SREG;\
  set_sleep_mode(mode);\
  cli();\
  if (expr) {\
    led_off(POWER_ACT_LED);\
    sleep_enable();\
    sei();\
    sleep_cpu();\
    sleep_disable();\
    led_on(POWER_ACT_LED);\
  }\
  SREG = __sreg_bak;\
} while (0)

#define sleep_while(mode,expr) do {\
  uint8_t __sreg_bak = SREG;\
  set_sleep_mode(mode);\
  while (1) {\
    cli();\
    if (expr) {\
      led_on(POWER_ACT_LED);\
      sleep_enable();\
      sei();\
      sleep_cpu();\
      sleep_disable();\
      led_off(POWER_ACT_LED);\
    }\
    else break;\
  }\
  SREG = __sreg_bak;\
} while (0)

#endif  // __SLEEP_UTIL_INTERFACE_H
