// avrtmon
// Host-side main source file
// Paolo Lucchesi - Wed 30 Oct 2019 07:24:56 PM CET
#include <stdio.h>
//#include "list.h"
#include "shell.h"


// Shell Command: echo
int shell_cmd_echo(int argc, char *argv[], void *storage) {
  for (int i=0; i < argc; ++i)
    puts(argv[i]);
  putchar('\n');
}
shell_command_t echo = {
  .name = "echo",
  .exec = shell_cmd_echo
};


int main(int argc, const char *argv[]) {
  // TODO: Define and handle command line arguments

  // TODO: Initialize communication interface

  // Initialize a list which will contain all the temperature DBs
  list_t *temp_dbs_storage = list_new();
  if (!temp_dbs_storage)
    return 1;

  // Launch the program command line
  shell_t *shell = shell_new("avrtmon> ", &echo, 1, temp_dbs_storage);
  shell_launch(shell);
  shell_delete(shell);

  return 0;
}
