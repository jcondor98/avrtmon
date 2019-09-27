// avrtmon
// AVR-side Configuration - Unit Test
// Paolo Lucchesi - Thu 05 Sep 2019 03:27:31 PM CEST
// NOTE: The test configuration shall be located in resources/config/test.csv
//   Do NOT change the test configuration!
#include <assert.h>
#include <string.h>  // mem??? functions

#include "test_framework.h"
#include "config.h"
#include "nvm_mock.h"


int main(int argc, const char *argv[]) {
  printf("avrtmon - AVR Configuration Unit Test\n\n");
  int ret;

  // Data structure to store config fields
  config_t _cfg_local;  // Do not use this directly...
  config_t *cfg_local = &_cfg_local;  // ... Use these intstead
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


  // Finally, initialize the configuration module (which lives in memory)
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
  ret = test_expr(memcmp(cfg_local, &nvm->config, sizeof(config_t)) == 0,
      "The obtained configuration data structure should be identical to the "
      "default one");

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
