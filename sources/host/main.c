// avrtmon
// Host-side main source file
// Paolo Lucchesi - Wed 30 Oct 2019 07:24:56 PM CET
#include <stdio.h>
#include <assert.h>

#include "communication.h"
#include "shell.h"


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
  // TODO: Handle errors
  assert(communication_init() == 0);

  // Initialize a list which will contain all the temperature DBs
  void *shell_storage = shell_storage_new();
  if (!shell_storage) {
    fputs("Error: unable to initialize shell storage\n", stderr);
    return 1;
  }

  // Launch the program command line
  shell_t *shell = shell_new("avrtmon> ", shell_commands,
      shell_commands_count, shell_storage);
  shell_launch(shell);
  shell_cleanup(shell);
  shell_delete(shell);

  return 0;
}
