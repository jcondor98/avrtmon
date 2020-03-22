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
  printf("NVM image dimension  -> %d\n", NVM_IMAGE_FULL_SIZE);
  printf("config_t size        -> %d\n", sizeof(nvm_image->config));
  printf("temperature_db_t size-> %d\n", sizeof(nvm_image->db));
  printf("DB capacity          -> %d\n", nvm_image->db.capacity);
  printf("TEMP_DB_CAPACITY     -> %d\n", TEMP_DB_CAPACITY);
  printf("NVM base address     -> %p\n", nvm);
  printf("sizeof(temperature_db_t) -> %d\n", sizeof(temperature_db_t));
  */


  const id_t test_items_limit =
    TEST_ITEMS_MAX < TEMP_DB_CAPACITY ? TEST_ITEMS_MAX : TEMP_DB_CAPACITY;
  printf("Test items limit -> %d\n", test_items_limit);

  putchar('\n');


  // Local database for the test unit
  // Assume that the database is already present in the NVM
  temperature_db_t local_db;
  temperature_t local_db_items[TEMP_DB_CAPACITY];

  // Aux variables
  int ret;


  mock_nvm_init();
  temperature_init(); // Initialize DB module
  test_expr(temperature_count() == 0, "DB should be initially empty");
  putchar('\n');


  printf("Testing temperature_register routine\n");
  for (id_t i=0; i < test_items_limit; ++i) {  // Fill the database
    // Works until temperature_t becomes a data structure
    ret = temperature_register((temperature_t) i);
    test_expr(temperature_count() == i+1,
        "DB item count should be consistent at iteration %d", i);
  }

  // Try to register too many temperatures (attempt should fail)
  for (id_t i=test_items_limit; i < TEMP_DB_CAPACITY; ++i)
    temperature_register((temperature_t) i);
  ret = temperature_register((temperature_t) 123);
  test_expr(ret != 0, "DB item should not be registered with no space left");
  test_expr(temperature_count() <= TEMP_DB_CAPACITY,
      "DB item count should not become bigger than its capacity");


  printf("\nTesting temperature_get routine\n");
  for (id_t i=0; i < test_items_limit; ++i) {  // Navigate across the entire DB
    temperature_t t;
    ret = temperature_get(i, &t);

    test_expr(ret == 0,
        "temperature_get() should be successful at iteration %d", i);
    test_expr(t == ((temperature_t) i),
        "Temperature should equal its ID (%d)", i);
  }


  printf("\nTesting temperature_fetch_entire_db routine\n");
  temperature_fetch_entire_db(&local_db, local_db_items);
  test_expr(local_db.capacity == temperature_capacity(),
      "'capacity' field of the fetched DB should be consistent");
  test_expr(local_db.used == temperature_count(),
      "'used' field of the fetched DB should be consistent");
  test_expr(local_db.items == local_db_items,
      "'items' should point to the choosen buffer in memory");

  for (id_t i=0; i < test_items_limit; ++i) {
    temperature_t t;
    temperature_get(i, &t);
    test_expr(local_db.items[i] == t,
        "Temperature of id %d should be consistent in the fetched DB", i);
  }

  printf("\nlocal_db.used        ->  %d", local_db.used);
  printf("\ntemperature_count()  ->  %d\n", temperature_count());


  printf("\nTesting temperature_db_reset routine\n");
  temperature_db_reset();
  test_expr(temperature_count() == 0, "DB should have been emptied");

  test_summary();
  return 0;
}
