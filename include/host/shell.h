// avrtmon
// Program shell (i.e. command line) - Source file
// Paolo Lucchesi - Thu 31 Oct 2019 02:12:01 AM CET
#ifndef __SHELL_MODULE_H
#define __SHELL_MODULE_H

// Different types of commands, in order of descending priority
// NOTE: CMD_TYPE_NONE must be the last value!
typedef enum COMMAND_TYPE_T {
  CMD_TYPE_ALL = 0, CMD_TYPE_BUILTIN, CMD_TYPE_EXTERNAL,CMD_TYPE_NONE
} command_type_t;

// Type definition for a shell command executor
// A shell command can have a return value (0 on success), and argv[0] is the
// name of the command itself, like command-line programs in the POSIX standard
// 'env' can be the shell storage in case of external commands and the entire
// shell context for built-ins, but the choice is up to the programmer
typedef int (*shell_command_f)(int argc, char *argv[], void *env);

// Type definition for a shell command
typedef struct _shell_command_s {
  char *name;
  char *help;  // Usage and brief description of the command
  shell_command_f exec;
} shell_command_t;

// Shell flags
typedef enum SHELL_FLAG_E {
  SH_SIG_EXIT = 1 << 0,
  SH_SCRIPT_MODE = 1 << 1,
  SH_EXIT_ON_ERR = 1 << 2
} shell_flag_t;

// Type definition for a shell
typedef struct _shell_s {
  char *prompt;
  shell_command_t *commands;
  shell_command_t *builtins;
  size_t commands_count;
  size_t builtins_count;
  void *storage;
  struct { // TODO: Complete to make this a full interface
    int (*compare)(const void *cmd1, const void *cmd2);
    shell_command_t* (*get)(const struct _shell_s *shell,
        const char *name, unsigned char type);
  } command_ops;
  unsigned char flags;
} shell_t;


// Create a new shell object, given a prompt and and a command array
// Returns a pointer to the new shell object on success, NULL otherwise
// The function will recreate a new commands array, so the one passed can be
// deallocated without any wanted effect. Same thing will be done for 'prompt'
shell_t *shell_new(const char *prompt, const shell_command_t *commands,
    size_t commands_count, const shell_command_t *builtins,
    size_t builtins_count, void *storage);

// Delete a shell (along with all the memory objects it uses)
void shell_delete(shell_t *shell);

// Launch a shell - Blocks until the user exits
void shell_loop(shell_t *shell, FILE *input);

// Process and execute a line
int shell_exec(shell_t *shell, const char *line);

// Execute a shell command, argv-style
int shell_execv(shell_t *shell, char *argv[]);

// Operations on shell flags
#define shell_flag_get(sh,flag) (((sh)->flags & (flag)) ? 1 : 0)
#define shell_flag_set(sh,flag) ((sh)->flags |= (flag))
#define shell_flag_clr(sh,flag) ((sh)->flags &= ~(flag))
#define shell_flag_tog(sh,flag) ((sh)->flags ^= (flag))


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
