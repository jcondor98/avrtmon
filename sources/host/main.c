// AVR Temperature Monitor -- Paolo Lucchesi
// Host-side main source file
#include <stdio.h>
#include <stdlib.h> // exit()
#include <unistd.h> // getopt

#include "communication.h"
#include "shell.h"
#include "debug.h"


// Import shell commands and specific utlity functions
extern shell_command_t *shell_commands;
extern shell_command_t *shell_builtins;
extern size_t shell_commands_count;
extern size_t shell_builtins_count;
extern void *shell_storage_new(void);
extern void shell_cleanup(shell_t *s);

// Print a help message for the program
static inline void print_usage(void) {
  printf("avrtmon -- AVR-based temperature monitor\n"
      "Usage: avrtmon [OPTION...]\n"
      "\n -c <avr-file-path>\n"
      "   Automatically connect at <avr-file-path>. Exit if the connention\n"
      "   could not be estabilished\n"
      "\n -s [script]\n"
      "   Script (i.e. non interactive) mode\n"
      "\n -h    Print a help message and exit\n"
      "\n"
  );
}

int main(int argc, char *argv[]) {
  // Command line arguments
  char *avr_dev_path = NULL; // Path to AVR device file

  // If script mode is enabled, keep track of a script path and file pointer
  FILE *script_file = NULL;
  char *script_path = NULL;

  // Handle command line arguments
  int opt;
  while ((opt = getopt(argc, argv, ":c:s:h")) >= 0) {
    switch (opt) {

      case 'c':
        avr_dev_path = optarg;
        break;

      case 'h':
        print_usage();
        exit(EXIT_SUCCESS);

      case ':':
        if (optopt == 's') script_file = stdin;
        else {
          fprintf(stderr, "Syntax error: Option -%c requires an argument\n", optopt);
          print_usage();
          exit(EXIT_FAILURE);
        }
        break;

      case '?':
        fprintf(stderr, "Syntax error: Unrecognized option -%c\n", optopt);
        print_usage();
        exit(EXIT_FAILURE);
    }
  }


  // Initialize communication and serial module
  err_check_exit(communication_init() != 0,
      "Could not initialize communication module");

  // Initialize a list which will contain all the temperature DBs
  void *shell_storage = shell_storage_new();
  err_check_exit(!shell_storage, "Could not initialize shell storage");

  // Launch the program command line
  shell_t *shell = shell_new("avrtmon> ",
      shell_commands, shell_commands_count, shell_storage);
  err_check_exit(!shell, "Could not initialize program shell");
  debug shell_print(shell);


  // Automatically connect if '-c' was specified
  if (avr_dev_path) {
    char *avr_connect_args[] = { "connect", avr_dev_path, NULL };
    if (shell_execv(shell, avr_connect_args) != 0)
      exit(EXIT_FAILURE);
  }

  // Handle script if present
  if (script_path && !(script_file = fopen(script_path, "r"))) {
      perror("Could not open script file");
      exit(EXIT_FAILURE);
  }
  if (script_file) shell_flag_set(shell, SH_SCRIPT_MODE | SH_EXIT_ON_ERR);


  // Main shell loop
  shell_loop(shell, script_file ? script_file : stdin);

  // Perform a clean exit from the program
  communication_cleanup();
  shell_cleanup(shell);
  shell_delete(shell);

  return 0;
}
