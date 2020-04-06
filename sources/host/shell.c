// avrtmon
// Program shell (i.e. command line) - Source file
// Paolo Lucchesi - Thu 31 Oct 2019 02:31:26 AM CET
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "host/shell.h"
#include "host/debug.h"


// Maximum length for a shell input line
#define SHELL_LINE_MAX_LEN 2046

// Shell default prompt
static const char *default_prompt = "> ";

// [AUX] Comparator for commands name
static int cmdcmp(const void *c1, const void *c2);

// [AUX] Retrieve a shell command given its name
static shell_command_t *shell_command_get(const shell_t *shell,
    const char *name, unsigned char type);


// Create a new shell object, given a prompt and and a command array
// Returns a pointer to the new shell object on success, NULL otherwise
// The function will recreate a new commands array, so the one passed can be
// deallocated without any unwanted effect. Same thing will be done for 'prompt'
shell_t *shell_new(const char *prompt, const shell_command_t *commands,
    size_t commands_count, const shell_command_t *builtins,
    size_t builtins_count, void *storage) {
  if (!commands || !commands_count || !builtins || !builtins_count)
    return NULL;

  // Create a new shell memory object
  shell_t *shell = malloc(sizeof(shell_t));
  if (!shell)
    goto handle_allocator_error;
  *shell = (shell_t) { 0 };

  // Allocate the commands and builtins array
  shell->commands = malloc(commands_count * sizeof(shell_command_t));
  if (!shell->commands)
    goto handle_allocator_error;
  shell->builtins = malloc(builtins_count * sizeof(shell_command_t));
  if (!shell->builtins)
    goto handle_allocator_error;

  // Take care of the command prompt
  shell->prompt = strdup(prompt ? prompt : default_prompt);
  if (!shell->prompt)
    goto handle_allocator_error;

  shell->commands_count = commands_count;
  shell->builtins_count = builtins_count;
  shell->storage = storage;
  shell->flags = 0;
  shell->command_ops.compare = cmdcmp;
  shell->command_ops.get = shell_command_get;

  // Sort the commands array
  memcpy(shell->commands, commands, commands_count * sizeof(shell_command_t));
  memcpy(shell->builtins, builtins, builtins_count * sizeof(shell_command_t));
  qsort(shell->commands, commands_count, sizeof(shell_command_t),
      shell->command_ops.compare);
  qsort(shell->builtins, builtins_count, sizeof(shell_command_t),
      shell->command_ops.compare);

  return shell;


handle_allocator_error:
  if (shell) {
    if (shell->prompt)   free(shell->prompt);
    if (shell->commands) free(shell->commands);
    if (shell->builtins) free(shell->builtins);
    free(shell);
  }

  return NULL;
}


// Delete a shell (along with all the memory objects it uses)
void shell_delete(shell_t *shell) {
  if (!shell) return;
  free(shell->commands);
  free(shell->builtins);
  free(shell->prompt);
  free(shell);
}


// Execute a fully parsed input line
static int _execute(shell_t *shell, int argc, char *argv[]) {
  // Attempt to get the command
  shell_command_t *cmd;
  int ret;

  if ((cmd = shell->command_ops.get(shell, argv[0], CMD_TYPE_BUILTIN)) != NULL) {
    ret = cmd->exec(argc, argv, shell);
    if (ret == 1) puts(cmd->help);  // i.e. command syntax error
  }

  else if ((cmd = shell->command_ops.get(shell, argv[0], CMD_TYPE_EXTERNAL)) != NULL) {
    ret = cmd->exec(argc, argv, shell->storage);
    if (ret == 1) puts(cmd->help);  // i.e. command syntax error
  }

  else { // Command does not exist
    printf("Command not found: %s\n", argv[0]);
    return 127; // Behave like POSIX shell
  }

  if (ret == 1 && cmd->help) puts(cmd->help);  // i.e. command syntax error
  return ret;
}



// Process an input line
// Returns the code returned by the executed command, or 127 if the command
// does not exist
static int _shell_exec(shell_t *shell, char *_line) {
  if (!shell->command_ops.get) return 1; // TODO: Return something like SH_INTERNAL_ERROR

  // argv-like buffer for the processed input line
  char *argv[SHELL_LINE_MAX_LEN / 2 + 1];
  int argc;

  // Tokenize the line
  // Take the first token. If it is null (i.e. an empty line was inserted),
  // just skip the command, as a POSIX shell would do
  argv[0] = strtok(_line, " \t\n");
  if (!argv[0]) return 0;

  // Get the command arguments (i.e. the other tokens)
  for (argc = 1; (argv[argc] = strtok(NULL, " \t\n")) != NULL; ++argc)
    ;

  return _execute(shell, argc, argv);
}


int shell_exec(shell_t *shell, const char *line) {
  if (!line) return 0;

  // Duplicate the string to tokenize it
  char _line[SHELL_LINE_MAX_LEN];
  strncpy(_line, line, SHELL_LINE_MAX_LEN);
  _line[SHELL_LINE_MAX_LEN-1] = '\0'; // Just to be sure

  return _shell_exec(shell, _line);
}


// Execute a shell command, argv-style
int shell_execv(shell_t *shell, char *argv[]) {
  if (!shell || !argv) return 0xFF; // TODO: Signal something like 'Shell internal error'
  int argc = 0;
  while (argv[argc])
    ++argc;
  return _execute(shell, argc, argv);
}


// Print the shell prompt
static inline void _prompt(const shell_t *shell) {
  if (!shell_flag_get(shell, SH_SCRIPT_MODE))
    fputs(shell->prompt, stdout);
}

// Launch a shell - Blocks until the user exits
void shell_loop(shell_t *shell, FILE *input) {
  if (!shell || !input) return;

  // Input line entered by the user
  char line[SHELL_LINE_MAX_LEN + 2]; // Consider newline plus null terminator

  // Main shell loop
  _prompt(shell); // Print first prompt
  while (1) {
    // Get a new input line
    if (!fgets(line, SHELL_LINE_MAX_LEN + 2, input)) { // No line was read
      if (feof(input))
        break;
      else if (ferror(input) && errno != EINTR)
        perror(__func__);
      continue;
    }

    // Attempt to execute command
    int ret = _shell_exec(shell, line);
    debug if (ret != 0) printf("Command returned error code %d\n", ret);

    // Catch exit signal
    if (shell_flag_get(shell, SH_SIG_EXIT))
      break;

    // Print the shell prompt again for the next input line
    _prompt(shell);
  }

  // User exited, print the exit message and return
  puts("\nSaluta Andonio");
}



// [AUX] Comparator for commands name to sort the commands array of the shell
static int cmdcmp(const void *c1, const void *c2) {
  if (!c1 && !c2) return 0;
  if (!c1) return -1;
  if (!c2) return  1;
  const shell_command_t *_c1 = c1, *_c2 = c2;
  return strcmp(_c1->name, _c2->name);
}


// [AUX] Retrieve a shell command given its name
static shell_command_t *shell_command_get(const shell_t *shell,
    const char *name, unsigned char type) {
  // Bad cast, I know, but 'name' is not going to be touched
  const shell_command_t cmd_dummy = { .name = (char*) name };
  shell_command_t *cmd;

  switch (type) {
    case CMD_TYPE_ALL:
      while (++type < CMD_TYPE_NONE)
        if ((cmd = shell_command_get(shell, name, type)) != NULL) return cmd;
      return NULL;

    case CMD_TYPE_BUILTIN:
      return bsearch(&cmd_dummy, shell->builtins, shell->builtins_count,
          sizeof(shell_command_t), shell->command_ops.compare);

    case CMD_TYPE_EXTERNAL:
      return bsearch(&cmd_dummy, shell->commands, shell->commands_count,
          sizeof(shell_command_t), shell->command_ops.compare);

    default: return NULL; // Unknown type
  }
}


#ifdef DEBUG
// [DEBUG] Print informations about a given shell
void shell_print(const shell_t *shell) {
  puts("\nPrinting shell");
  if (!shell) {
    printf("Shell pointer is NULL");
    return;
  }

  printf("Prompt: %s\n", shell->prompt);
  printf("Storage: %s\n", shell->storage ? "Present" : "Absent");
  printf("Commands number: %zu\n", shell->commands_count);

  if (shell->commands_count) {
    puts("Commands:\n");
    for (size_t i=0; i < shell->commands_count; ++i)
      printf("  - %s\n", shell->commands[i].name);
  }

  putchar('\n');
}
#endif
