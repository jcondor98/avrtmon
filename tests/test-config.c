// AVR Temperature Monitor -- Paolo Lucchesi
// AVR-side Configuration - Test Unit
// NOTE: The test configuration shall be located in resources/config/test.csv
//   Do NOT change the test configuration!
#include <assert.h>
#include <string.h>  // mem??? functions

#include "test_framework.h"
#include "config.h"
#include "nvm.h"


// Inspect a chunk of memory
static void inspect(const void *src, size_t size) {
  printf("0x");
  for (size_t i=0; i < size; ++i) {
    unsigned char byte = *(((unsigned char*) src) + i);
    printf(byte & 0xF0 ? " %2hhx" : " 0%1hhx", byte);
  }
}


int main(int argc, const char *argv[]) {
  printf("avrtmon - AVR Configuration Unit Test\n");
  int ret, expr;

  nvm_mock_init();

  // Data structure to store config fields
  config_t cfg_local[1] = { { 0 } };
  void *cfg_local_raw = (void*) cfg_local;

  // Use a buffer as a temporary storage for a configuration field value
  // We will make this as big as an entire configuration data structure, so
  // the field will surely fit into it
  config_t _field_tmp;  // Do not use this directly...
  void *field_tmp = &_field_tmp;  // ... Use this instead

  // Dynamically keep track of configuration fields
  uint8_t cfg_field_offsets[CONFIG_FIELD_COUNT];
  for (unsigned field=0; field < CONFIG_FIELD_COUNT; ++field)
    cfg_field_offsets[field] = config_get_offset(field);


  printf("\nTesting 'config_fetch'\n");
  ret = config_fetch();
  test_expr(ret == 0, "Configuration data should be fetched successfully");

  printf("\nTesting 'config_get'\n");
  for (unsigned field=0; field < CONFIG_FIELD_COUNT; ++field) {
    ret = config_get(field, cfg_local_raw + cfg_field_offsets[field]);
    test_expr(ret == 0, "Field %d should be get successfully", field);
  }

  printf("\nTesting 'config_get' against malformed parameters\n");
  ret = config_get(CONFIG_FIELD_COUNT, field_tmp);
  test_expr(ret != 0, "An inexistent field should not be get");
  ret = config_get(0, NULL);  // Note that 0 is a valid field ID
  test_expr(ret != 0, "An existent field should not be get with an inexistent "
      "destination pointer");

  printf("\nTesting obtained (default) values\n");
  expr = memcmp(cfg_local, &nvm_image->config, sizeof(config_t)) == 0;
  ret = test_expr(expr, "The obtained configuration data structure should be "
      "identical to the default one");
  if (!expr) {
    printf("Local config data: ");
    inspect(cfg_local, sizeof(config_t));
    printf("\nNVM config data:   ");
    inspect(&nvm_image->config, sizeof(config_t));
    putchar('\n');
  }

  printf("\nTesting 'config_set'\n");
  // We flip all bits to 1 for every configuration field
  memset(field_tmp, 0xFF, sizeof(*field_tmp));
  for (unsigned field=0; field < CONFIG_FIELD_COUNT; ++field) {
    unsigned field_size = config_get_size(field);
    memset(cfg_local, 0x00, field_size);
    ret = config_set(field, field_tmp);
    test_expr(ret == 0, "Field %d should be set successfully", field);
    config_get(field, cfg_local);
    test_expr(memcmp(field_tmp, cfg_local, field_size) == 0,
        "Field %d should have been effectively set", field);
  }

  printf("\nTesting 'config_set' against malformed values\n");
  ret = config_set(CONFIG_FIELD_COUNT, field_tmp);
  test_expr(ret != 0, "An inexistent field should not be set");
  ret = config_set(0, NULL);  // Note that 0 is a valid field ID
  test_expr(ret != 0, "An existent field should not be set with an inexistent "
      "destination pointer");


  // End unit test
  test_summary();
  return 0;
}
