// avrtmon
// Program shell (i.e. command line) - Source file
// Paolo Lucchesi - Thu 31 Oct 2019 02:12:01 AM CET
#ifndef __SHELL_MODULE_H
#define __SHELL_MODULE_H

// Type definition for a shell command executor
// A shell command can have a return value (0 on success), and argv[0] is the
// name of the command itself, like command-line programs in the POSIX standard
typedef int (*shell_command_f)(int argc, char *argv[], void *storage);

// Type definition for a shell command
typedef struct _shell_command_s {
  char *name;
  char *help;  // Usage and brief description of the command
  shell_command_f exec;
} shell_command_t;

// Type definition for a shell
typedef struct _shell_s {
  char *prompt;
  shell_command_t *commands;
  size_t commands_count;
  void *storage;
} shell_t;


// Create a new shell object, given a prompt and and a command array
// Returns a pointer to the new shell object on success, NULL otherwise
// The function will recreate a new commands array, so the one passed can be
// deallocated without any wanted effect. Same thing will be done for 'prompt'
shell_t *shell_new(const char *prompt, const shell_command_t *commands,
    size_t commands_count, void *storage);

// Delete a shell (along with all the memory objects it uses)
void shell_delete(shell_t *shell);

// Launch a shell - Blocks until the user exits
void shell_launch(shell_t *shell);


// Exit with an error from a shell command, printing a message
// NOTE: Use only within a shell command!
#define sh_error(err_ret, err_fmt, ...) do {\
    fprintf(stderr, "%s: ", argv[0]);\
    fprintf(stderr, err_fmt __VA_OPT__(,) __VA_ARGS__);\
    fputc('\n', stderr);\
    return err_ret;\
} while (0)

// Same as above, but print the error message and exit only if 'expr' is true
#define sh_error_on(expr, err_ret, err_fmt, ...) do {\
  if (expr) {\
    fprintf(stderr, "%s: ", argv[0]);\
    fprintf(stderr, err_fmt __VA_OPT__(,) __VA_ARGS__);\
    fputc('\n', stderr);\
    return err_ret;\
  }\
} while (0)


// [DEBUG] Print informations about a given shell
#ifdef DEBUG
void shell_print(const shell_t *shell);
#endif

#endif    // __SHELL_MODULE_H
