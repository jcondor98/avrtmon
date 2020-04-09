// AVR Temperature Monitor -- Paolo Lucchesi
// Test Framework - Head file
#ifndef __TEST_FRAMEWORK_H
#define __TEST_FRAMEWORK_H
#include <stdio.h>
#include <stdarg.h>

// Output stream to which test messages will be written
#define OUT_STREAM stdout

// Is a test passed or failed?
typedef enum TEST_RESULT_E {
  TEST_PASSED = 0, TEST_FAILED = 1
} test_result_t;

// Text an expression which can be evaluated as true (i.e. !0) or false (i.e. 0)
// The test is passed if the expression evaluates to true
// Returns 1 if the test passes, 0 otherwise
// Also accepts NULL as a description of the test case
test_result_t test_expr(int expr, const char *desc, ...);

// Return the total number of performed tests
unsigned test_count(void);

// Return the number of passed/failed tests
unsigned test_count_passed(void);
unsigned test_count_failed(void);

// Print a brief summary of all the performed tests
void test_summary(void);


// The test framework is shipped with some printf-like function which can print
// colored stuff if the environment supports it
// For testing purposes, you do not need to use it directly 

// Colors for colored output (e.g. when using fprintf_colored
#define N_COLOR_VALUES 15
typedef enum ANSI_COLOR_E {
  C_NONE            =   0,
  C_RED             =   31,
  C_GREEN           =   32,
  C_YELLOW          =   33,
  C_BLUE            =   34,
  C_MAGENTA         =   35,
  C_CYAN            =   36,
  C_WHITE           =   37,
  C_BRIGHT_RED      =   91,
  C_BRIGHT_GREEN    =   92,
  C_BRIGHT_YELLOW   =   93,
  C_BRIGHT_BLUE     =   94,
  C_BRIGHT_MAGENTA  =   95,
  C_BRIGHT_CYAN     =   96,
  C_BRIGHT_WHITE    =   97
} ansi_color_t;

// Color escape sequence (to start printing colored stuff)
#define COLOR_ESCAPE_SEQ_FMT "\x1b[%dm"

// 'fprintf' colored version
// Return the number of written bytes, or a negative number on error
int fprintf_colored(FILE *out, ansi_color_t color, const char *format, ...);

// Same as fprintf_colored, but automatically prints to OUT_STREAM
#define printf_colored(color, fmt, ...) \
  fprintf_colored(OUT_STREAM, color, fmt __VA_OPT__(,) __VA_ARGS__)

// 'fprintf' colored version
// Return the number of written bytes, or a negative number on error
int vfprintf_colored(FILE *out, ansi_color_t color,
    const char *format, va_list args);

// Same as vfprintf_colored, but automatically prints to OUT_STREAM
#define vprintf_colored(color, fmt, args) \
  vfprintf_colored(OUT_STREAM, color, args)

#endif    // __TEST_FRAMEWORK_H
