// AVR Temperature Monitor -- Paolo Lucchesi
// Debug and error handling facilities - Host side
#ifndef __DEBUG_H
#define __DEBUG_H
#include <stdio.h>

#ifndef DEBUG
#define DEBUG 0
#endif

// If we just define DEBUG without a value, it is redefined to 1 (if non-zero)
#if defined(DEBUG) && DEBUG != 0
#undef DEBUG
#define DEBUG 1
#endif

// Use as 'debug action' or 'debug { many actions }'
#define debug if (DEBUG)


// Error handling facilities

// printf for stderr
#define eprintf(fmt, ...) fprintf(stderr, fmt __VA_OPT__(,) __VA_ARGS__)

// Log an error, works like eprintf but print also the function name
#define err_log(err_fmt, ...) do {\
  eprintf("%s: ", __func__);\
  eprintf(err_fmt __VA_OPT__(,) __VA_ARGS__);\
  fputc('\n', stderr);\
} while (0)

// If 'expr' is true, print 'err_fmt' (with arguments) and return 'err_ret'
#define err_check(expr, err_ret, err_fmt, ...) do {\
  if (expr) {\
    eprintf("%s: ", __func__);\
    eprintf(err_fmt __VA_OPT__(,) __VA_ARGS__);\
    fputc('\n', stderr);\
    return err_ret;\
  }\
} while (0)

// Like 'err_check' where 'expr' is assumed to be true
#define error(err_ret, err_fmt, ...)\
  err_check(1, err_ret, err_fmt __VA_OPT__(,) __VA_ARGS__)

// If 'expr' is true, perror and return 'err_ret'
#define err_check_perror(expr, err_ret) do {\
  if (expr) {\
    perror(__func__);\
    return err_ret;\
  }\
} while (0);

// If 'expr' is true, print 'err_fmt' and exit from the program
#define err_check_exit(expr, err_fmt, ...) do {\
  if (expr) {\
    eprintf("%s: ", __func__);\
    eprintf(err_fmt __VA_OPT__(,) __VA_ARGS__);\
    fputc('\n', stderr);\
    exit(EXIT_FAILURE);\
  }\
} while (0)

#endif  // __DEBUG_H
