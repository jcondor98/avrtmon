// avrtmon
// Test Framework - Source file
// Paolo Lucchesi - Sat 24 Aug 2019 07:16:02 PM CEST
#include "test_framework.h"

// Register passed and failed tests
static unsigned test_passed;
static unsigned test_failed;


// Text an expression which can be evaluated as true (i.e. !0) or false (i.e. 0)
// The test is passed if the expression evaluates to true
// Returns 1 if the test passes, 0 otherwise
// Also accepts NULL as a description of the test case
test_result_t test_expr(int expr, const char *desc, ...) {
  if (expr) {
    printf_colored(C_BRIGHT_GREEN, "[PASSED] ");
    ++test_passed;
  }
  else {
    printf_colored(C_BRIGHT_RED, "[FAILED] ");
    ++test_failed;
  }

  va_list args;
  va_start(args, desc);
  vprintf(desc, args);
  va_end(args);

  putc('\n', OUT_STREAM);
  return expr ? 1 : 0;
}

// Return the total number of performed tests
unsigned test_count(void) { return test_passed + test_failed; }

// Return the number of passed/failed tests
unsigned test_count_passed(void) { return test_passed; }
unsigned test_count_failed(void) { return test_failed; }


// Print a brief summary of all the performed tests
void test_summary(void) {
  printf_colored(C_BRIGHT_BLUE, "\n\n| TEST SUMMARY\n\\___________________\n");
  printf("TOTAL:  %u\nPASSED: %u\nFAILED: %u\n\n",
      test_passed + test_failed, test_passed, test_failed);
}


// Print a coloured string - wrapper to vfprintf
int vfprintf_colored(FILE *out, ansi_color_t color,
    const char *format, va_list args) {

  // Check if format and color exist
  if (!format || color < 31 || (color > 37 && color < 90) || color > 97)
    return 0;

  // Begin colored output
  if (color != C_NONE)
    fprintf(out, COLOR_ESCAPE_SEQ_FMT, color);

  int ret = vfprintf(out, format, args); // Print format

  // Reset color
  if (color != C_NONE)
    fprintf(out, COLOR_ESCAPE_SEQ_FMT, 0);  // i.e. color reset sequence

  return ret; // Return written bytes
}


// Print a coloured string - wrapper to vfprintf_colored
int fprintf_colored(FILE *out, ansi_color_t color, const char *format, ...) {
  // Initialize va list
  va_list args;
  va_start(args, format);

  int ret = vfprintf_colored(out, color, format, args);

  va_end(args);
  return ret;   // Return number of written bytes
}
