// avrtmon
// Temperature database - Head file
// Paolo Lucchesi - Test Unit
#include <stdio.h>
#include "test_framework.h"

#include "temperature.h"


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
  test_expr(temperature_count() == 0, "DB should be initially empty");
  putchar('\n');


  printf("Testing temperature_register routine\n");
  for (id_t i=0; i < TEMP_DB_CAPACITY; ++i) {  // Fill the database
    // Works until temperature_t becomes a data structure
    ret = temperature_register((temperature_t) i);
    test_expr(temperature_count() == i+1,
        "DB item count should be consistent at iteration %d", i);
  }

  // Try to register another temperature (attempt should fail)
  ret = temperature_register((temperature_t) 123);
  test_expr(ret != 0, "DB item should not be registered with no space left");
  test_expr(temperature_count() == TEMP_DB_CAPACITY,
      "DB item count should not become bigger than its capacity");


  printf("\nTesting temperature_get routine\n");
  for (id_t i=0; i < TEMP_DB_CAPACITY; ++i) {  // Navigate across the entire DB
    temperature_t t;
    ret = temperature_get(i, &t);

    test_expr(ret == 0,
        "temperature_get() should be successful at iteration %d", i);
    test_expr(t == ((temperature_t) i),
        "Temperature should equal its ID (%d)", i);
  }


  printf("\nTesting temperature_fetch_entire_db routine\n");
  temperature_fetch_entire_db(&local_db);
  test_expr(local_db.used == temperature_count(),
      "'used' field of the fetched DB should be consistent");
  test_expr(local_db.capacity == temperature_capacity(),
      "'capacity' field of the fetched DB should be consistent");

  for (id_t i=0; i < TEMP_DB_CAPACITY; ++i) {
    temperature_t t;
    temperature_get(i, &t);
    test_expr(local_db.items[i] == t,
        "Temperature of id %d should be consistent in the fetched DB", i);
  }


  printf("\nTesting temperature_db_reset routine\n");
  temperature_db_reset();
  test_expr(temperature_count() == 0, "DB should have been emptied");

  test_summary();
  return 0;
}
