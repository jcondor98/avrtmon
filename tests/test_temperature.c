// avrtmon
// Temperature database - Head file
// Paolo Lucchesi - Test Unit
#include <stdio.h>
#include "temperature.h"


// Test an expression which can be evaluated as true/false
// Returns 1 if the expression is evaluated as true, 0 otherwise
// Conceptually, a test passes if 'expr' is true
static int test_expr(int expr, const char *assertion) {
  printf("[%s] %s\n", expr ? "PASSED" : "FAILED", assertion);
  return expr ? 1 : 0;
}


int main(int argc, const char *argv[]) {
  printf("avrtmon - Temperature Database Test Unit\n\n");
  printf("TEMP_DB_CAPACITY -> %d\n", TEMP_DB_CAPACITY);
  printf("TEMP_DB_OFFSET   -> %d\n", TEMP_DB_OFFSET);
  printf("sizeof(temperature_db_t) -> %d\n", sizeof(temperature_db_t));
  putchar('\n');

  // Local database for the test unit
  // Assume that the database is already present in the NVM
  temperature_db_t local_db;

  // Aux variables
  int ret;


  mock_nvm_init_for_temperature_db();
  temperature_init(); // Initialize DB module
  test_expr(temperature_count() == 0, "DB should be empty");
  putchar('\n');


  printf("Testing temperature_register routine\n");
  for (id_t i=0; i < TEMP_DB_CAPACITY; ++i) {  // Fill the database
    // Works until temperature_t becomes a data structure
    ret = temperature_register((temperature_t) i);
    test_expr(temperature_count() == i+1,
        "DB item count should be consistent");
  }

  // Try to register another temperature (attempt should fail)
  ret = temperature_register((temperature_t) 123);
  test_expr(ret != 0, "Function call should not be successful");
  test_expr(temperature_count() == TEMP_DB_CAPACITY,
      "DB item count should not change");


  printf("\nTesting temperature_get routine\n");
  for (id_t i=0; i < TEMP_DB_CAPACITY; ++i) {  // Navigate across the entire DB
    temperature_t t;
    ret = temperature_get(i, &t);
    test_expr(ret == 0, "Function call should be successful");
    test_expr(t == ((temperature_t) i), "Temperature should equal its ID");
  }


  printf("\nTesting temperature_fetch_entire_db routine\n");
  temperature_fetch_entire_db(&local_db);
  test_expr(local_db.used == temperature_count(),
      "'used' field should be consistent");
  test_expr(local_db.capacity == temperature_capacity(),
      "'capacity' field should be consistent");

  for (id_t i=0; i < TEMP_DB_CAPACITY; ++i) {
    temperature_t t;
    temperature_get(i, &t);
    test_expr(local_db.items[i] == t, "Temperature should be consistent");
  }


  printf("\nTesting temperature_db_reset routine\n");
  temperature_db_reset();
  test_expr(temperature_count() == 0, "DB should have been emptied");


  return 0;
}
