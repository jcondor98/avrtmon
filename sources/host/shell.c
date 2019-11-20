// avrtmon
// Program shell (i.e. command line) - Source file
// Paolo Lucchesi - Thu 31 Oct 2019 02:31:26 AM CET
#define _POSIX_C_SOURCE 200809L  // So we can use strdup() with ANSI C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"


// Maximum length for a shell input line
#define SHELL_LINE_MAX_LEN 2046

// Shell default prompt
static const char *default_prompt = "> ";


// [AUX] Comparator for commands name to sort the commands array of the shell
static int cmdcmp(const void *c1, const void *c2);

// [AUX] Retrieve a shell command given its name
static shell_command_t *shell_command_get(const shell_t *shell, const char *name);


// Create a new shell object, given a prompt and and a command array
// Returns a pointer to the new shell object on success, NULL otherwise
// The function will recreate a new commands array, so the one passed can be
// deallocated without any wanted effect. Same thing will be done for 'prompt'
shell_t *shell_new(const char *prompt, const shell_command_t *commands,
    size_t commands_count, void *storage) {
  if (!commands || commands_count == 0) return NULL;

  // Create a new shell memory object
  shell_t *shell = malloc(sizeof(shell_t));
  if (!shell)
    goto handle_allocator_error;

  // Setup a sorted-by-name commands array
  shell->commands = malloc(commands_count * sizeof(shell_command_t));
  if (!shell->commands)
    goto handle_allocator_error;
  memcpy(shell->commands, commands, commands_count * sizeof(shell_command_t));
  qsort(shell->commands, commands_count, sizeof(shell_command_t), cmdcmp);

  // Take care of the command prompt
  shell->prompt = strdup(prompt ? prompt : default_prompt);
  if (!shell->prompt)
    goto handle_allocator_error;

  shell->commands_count = commands_count;
  shell->storage = storage;

  return shell;


handle_allocator_error:
  if (shell) {
    if (shell->prompt)   free(shell->prompt);
    if (shell->commands) free(shell->commands);
    free(shell);
  }

  return NULL;
}


// Delete a shell (along with all the memory objects it uses)
void shell_delete(shell_t *shell) {
  if (!shell) return;
  free(shell->commands);
  free(shell->prompt);
  free(shell);
}


// Launch a shell - Blocks until the user exits
void shell_launch(shell_t *shell) {
  if (!shell) return;

  // Input line entered by the user
  char line[SHELL_LINE_MAX_LEN + 2]; // Consider newline plus null terminator

  // argv-like buffer for a single processed input line
  char *argv[SHELL_LINE_MAX_LEN / 2 + 1];
  int argc;

  // Main shell loop -- Read an entire line inserted by the user
  fputs(shell->prompt, stdout);
  while (fgets(line, SHELL_LINE_MAX_LEN + 2, stdin)) {
    // Tokenize the line
    // Take the first token. If it is null (i.e. an empty line was inserted),
    // just skip the command, as a POSIX shell would do
    argv[0] = strtok(line, " \t\n");
    if (!argv[0]) {
      fputs(shell->prompt, stdout);
      continue;
    }

    // Get the command arguments (i.e. the other tokens)
    for (argc = 1; (argv[argc] = strtok(NULL, " \t\n")) != NULL; ++argc)
      ;

    // First, determine if the command is a built-in
    if (strcmp(argv[0], "help") == 0) {  // 'help'
      // Generic use of 'help'
      if (argc == 1)
        for (size_t i=0; i < shell->commands_count; ++i) {
          fputs(shell->commands[i].name, stdout);
          putchar('\n');
        }

      // 'help' takes a command name as its sole argument
      else if (argc == 2) {
        shell_command_t *cmd = shell_command_get(shell, argv[1]);
        if (cmd) {
          if (cmd->help) {
            fputs(cmd->help, stdout);
            putchar('\n');
          }
          else printf("No help for command %s\n", cmd->name);
        }
        else printf("Unknown command: %s\n", argv[1]);
      }

      // No more than an argument is accepted
      else fputs("Bad use of 'help'\nUsage: help [command]\n", stdout);
    }

    else if (strcmp(argv[0], "exit") == 0)  // 'exit'
      break;

    else { // Search and eventually launch the command (if found)
      shell_command_t *cmd = shell_command_get(shell, argv[0]);
      if (cmd) {
        int ret = cmd->exec(argc, argv, shell->storage);
        if (ret != 0)
          fprintf(stderr, "Command '%s' exited with status %d\n", cmd->name, ret);
      }
      else printf("Command not found: %s\n", argv[0]);
    }

    // Print the shell prompt again for the next input line
    fputs(shell->prompt, stdout);
  }

  // User exited, print the exit message and return
  fputs("\nSaluta Andonio\n", stdout);
}



// [AUX] Comparator for commands name to sort the commands array of the shell
static int cmdcmp(const void *c1, const void *c2) {
  if (!c1) return -1;
  if (!c2) return  1;
  const shell_command_t *_c1 = c1, *_c2 = c2;
  return strcmp(_c1->name, _c2->name);
}

// [AUX] Retrieve a shell command given its name
static shell_command_t *shell_command_get(const shell_t *shell, const char *name) {
  // Bad cast, I know, but 'name' is not going to be touched
  const shell_command_t cmd_dummy = { .name = (char*) name };
  return bsearch(&cmd_dummy, shell->commands, shell->commands_count,
      sizeof(shell_command_t), cmdcmp);
}
