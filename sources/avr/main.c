// avrtmon
// Main routine
// Paolo Lucchesi - Fri 27 Sep 2019 07:18:30 PM CEST
#include "temperature.h"
#include "communication.h"
#include "config.h"
#include "command.h"


int main(int argc, const char *argv[]) {
  // Initialize all modules
  config_fetch();
  temperature_init();
  command_init();
  com_init();

  // Main application loop
  while (1) {
    com_handler(); // Check for incoming packets
    //temperature_handler();   // Check for temperatures ready to be registered
  }
}

