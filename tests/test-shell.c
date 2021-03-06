// AVR Temperature Monitor -- Paolo Lucchesi
// Program shell (i.e. command line) - Test Unit
#include <stdio.h>
#include "shell.h"


// Shell Command: echo
int shell_cmd_echo(int argc, char *argv[], void *storage) {
  for (int i=1; i < argc; ++i) {
    fputs(argv[i], stdout);
    putchar(i == argc-1 ? '\n' : ' ');
  }
  return 0;
}

shell_command_t echo = {
  .name = "echo",
  .help = "Usage: echo [arg1 arg2 ...]\nRepeat what you write",
  .exec = shell_cmd_echo
};


int main(int argc, const char *argv[]) {
  shell_t *shell = shell_new("> ", &echo, 1, NULL);
  if (!shell) {
    printf("Couldn't create new shell memory object\n");
    return 1;
  }

  // Launch the shell
  printf("Launching the test shell\n");
  shell_launch(shell);

  shell_delete(shell);
  return 0;
}
