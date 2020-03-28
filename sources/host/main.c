// avrtmon
// Host-side main source file
// Paolo Lucchesi - Wed 30 Oct 2019 07:24:56 PM CET
#include <stdio.h>
#include <stdlib.h> // exit()

#include "communication.h"
#include "shell.h"
#include "debug.h"


// Import shell commands and specific utlity functions
extern shell_command_t *shell_commands;
extern size_t shell_commands_count;
extern void *shell_storage_new(void);
extern void shell_cleanup(shell_t *s);


int main(int argc, const char *argv[]) {
  // TODO: Define and handle command line arguments (or remove this comment)
  if (argc > 1)
    puts("Warning: no command line arguments are supported at the moment");

  // Initialize communication and serial module
  err_check_exit(communication_init() != 0,
      "Could not initialize communication module");

  // Initialize a list which will contain all the temperature DBs
  void *shell_storage = shell_storage_new();
  err_check_exit(!shell_storage, "Could not initialize shell storage");

  // Launch the program command line
  shell_t *shell = shell_new("avrtmon> ", shell_commands,
      shell_commands_count, shell_storage);
  err_check_exit(!shell, "Could not initialize program shell");

  // Main shell loop
  shell_launch(shell);

  // Perform a clean exit from the program
  communication_cleanup();
  shell_cleanup(shell);
  shell_delete(shell);

  return 0;
}
