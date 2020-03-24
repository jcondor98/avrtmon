// avrtmon
// Temperature database - Head file
// Paolo Lucchesi - Test Unit
#include <stdio.h>
#include "test_framework.h"
#include "nvm.h"

#include "temperature.h"


#define TEST_ITEMS_MAX 3  // Avoid long iterations for repetitive tests

int main(int argc, const char *argv[]) {
  printf("avrtmon - Temperature Database Test Unit\n\n");

  /* Debug info which has been used for the unit test itself
  printf("NVM image address    -> %p\n", nvm_image);
  printf("config_t size        -> %d\n", sizeof(nvm_image->config));
  printf("temperature_db_t size-> %d\n", sizeof(nvm_image->db));
  printf("DB capacity          -> %d\n", nvm_image->db.capacity);
  printf("TEMP_DB_CAPACITY     -> %d\n", TEMP_DB_CAPACITY);
  printf("NVM base address     -> %p\n", nvm);
  printf("sizeof(temperature_db_t) -> %d\n", sizeof(temperature_db_t));
  */

  int ret;
  const temperature_id_t test_items_limit = TEST_ITEMS_MAX;
  printf("Test items limit -> %d\n\n", test_items_limit);

  nvm_mock_init();


  // Initialize temperatures DB module
  test_expr(temperature_init() == 0,
      "Temperatures DB module should be succesfully initialized");
  test_expr(temperature_count_all() == 0,
      "Total number of registered temperatures should be initially null");
  test_expr(temperature_count(0) == 0,
      "First database should contain no temperatures");


  printf("\nTesting temperature_register routine\n");
  for (temperature_id_t i=0; i < test_items_limit; ++i) {  // Fill the database
    // Works until temperature_t becomes a data structure
    test_expr(temperature_register((temperature_t) i) == 0,
        "A new temperature should be successfully registered");
    test_expr(temperature_count(0) == i+1,
        "DB item count should be consistent at iteration %d", i);
  }


  printf("\nTesting temperature_get routine\n");
  for (temperature_id_t i=0; i < test_items_limit; ++i) {  // Navigate across the entire DB
    temperature_t t;
    ret = temperature_get(0, i, &t);

    test_expr(ret == 0,
        "temperature_get() should be successful at iteration %d", i);
    test_expr(t == ((temperature_t) i),
        "Temperature should equal its ID (%d)", i);
  }

  printf("\nTesting temperature_get_bulk routine\n");
  temperature_t temps_buf[test_items_limit];
  test_expr(temperature_get_bulk(0, 0, test_items_limit, temps_buf) == test_items_limit,
        "temperature_get_bulk should get all the temperatures");
  test_expr(temperature_get_bulk(0, 1, test_items_limit, temps_buf) == test_items_limit-1,
        "temperature_get_bulk should not get more temperatures than the present ones");


  // Create another DB
  printf("\nTesting temperature_db_new routine\n");
  test_expr(temperature_db_new() == 0, "New DB should be created successfully");

  // Register a temperature in the new DB
  test_expr(temperature_register(0) == 0,
      "A new temperature should be successfully registered");
  test_expr(temperature_count(0) == test_items_limit,
      "First created DB should not be touched");
  test_expr(temperature_count(1) == 1,
      "Second created DB should contain a new temperature");


  printf("\nTesting temperature_db_reset routine\n");
  temperature_db_reset();
  test_expr(temperature_count_all() == 0, "DBs should have been emptied");


  test_summary();
  return 0;
}
