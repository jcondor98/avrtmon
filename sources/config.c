// avrtmon
// AVR-side Configuration - Source file
// Paolo Lucchesi - Tue 27 Aug 2019 06:58:59 PM CEST
#include <string.h> // memcpy
#include <stddef.h> // offsetof

#include "config.h"

#ifdef AVR
#include "nvm.h"
#endif


// Stuff common to host and AVR

// Data type to store size and relative offset of a field
typedef struct _config_field_accessor_s {
  uint8_t size;
  uint8_t offset;
} config_field_accessor_t;

// Store metadata to dynamically access configuration fields
static const config_field_accessor_t cfg_accessors[CONFIG_FIELD_COUNT] = {
  { .size = sizeof(uint16_t), .offset = offsetof(config_t, temperature_timer_resolution) },
  { .size = sizeof(uint16_t), .offset = offsetof(config_t, temperature_timer_interval) },
  { .size = sizeof(uint8_t), .offset = offsetof(config_t, lmsensor_pin) },
  { .size = sizeof(uint8_t), .offset = offsetof(config_t, btn_debounce_time) },
  { .size = sizeof(uint8_t), .offset = offsetof(config_t, start_pin) },
  { .size = sizeof(uint8_t), .offset = offsetof(config_t, stop_pin) }
};

// Get the size of a single field
uint8_t config_get_size(config_field_t field) {
  return field >= CONFIG_FIELD_COUNT ? 0 : cfg_accessors[field].size;
}


// AVR-side stuff
#ifdef AVR

// Configuration data which lives in memory
static config_t config;
static void *config_raw = &config;

// Address to the configuration data in the NVM
#define NVM_ADDR_CONFIG ((void*) &nvm_image->config)


// Get the value of a configuration field
// Returns 0 on success, 1 otherwise
uint8_t config_get(config_field_t field_id, void *dest) {
  if (!dest || field_id >= CONFIG_FIELD_COUNT)
    return 1;
  memcpy(dest, config_raw + cfg_accessors[field_id].offset,
      cfg_accessors[field_id].size);
  return 0;
}

// Set the value of a configuration field
// Returns 0 on success, 1 otherwise
uint8_t config_set(config_field_t field_id, const void *value) {
  if (!value || field_id >= CONFIG_FIELD_COUNT)
    return 1;
  memcpy(config_raw + cfg_accessors[field_id].offset, value,
      cfg_accessors[field_id].size);
  return 0;
}

// Fetch the configuration data structure from the NVM
// 'src' is the offset in the NVM of the config data structure to fetch
uint8_t config_fetch(void) {
  nvm_read(config_raw, NVM_ADDR_CONFIG, sizeof(config_t));
  return 0;
}

// Save the entire configuration data structure living in memory to the NVM
// Returns 0 on success, 1 on error
uint8_t config_save(void) {
  nvm_update(NVM_ADDR_CONFIG, config_raw, sizeof(config_t));
  return 0;
}

// Same as 'config_save', but apply changes only to a single field
// Returns 0 on success, 1 on error
uint8_t config_save_field(config_field_t field) {
  if (field >= CONFIG_FIELD_COUNT)
    return 1;
  nvm_update(NVM_ADDR_CONFIG + cfg_accessors[field].offset,
      config_raw + cfg_accessors[field].offset, cfg_accessors[field].size);
  return 0;
}


// Host-side stuff
#else

// TODO: (maybe) Optimize making this a hash table
static const char *_config_field_str[] = {
  "temperature_timer_resolution",
  "temperature_timer_interval",
  "lmsensor_pin",
  "btn_debounce_time",
  "start_pin",
  "stop_pin"
};

// Get a string representing a config field (by config field id)
// Returns a pointer to a string relative to the field id, or NULL on failure
const char *config_field_str(config_field_t field) {
  return (field < CONFIG_FIELD_COUNT) ? _config_field_str[field] : NULL;
}

// Get a config field id given its id
// On success, 0 is returned and the config field value is written to 'dest'
// On failure, 1 is returned and 'dest' is not touched
// TODO: (maybe) Optimize making 'config_field_str' a hash table
int config_field_id(const char *desc, config_field_t *dest) {
  for (config_field_t f=0; f < CONFIG_FIELD_COUNT; ++f)
    if (strcmp(desc, _config_field_str[f]) == 0) {
      *dest = f;
      return 0;
    }
  return 1;
}

#endif  // AVR


// The functions below shall be used only for testing
#ifdef TEST

// Get the offset of a single field
uint8_t config_get_offset(config_field_t field) {
  return field >= CONFIG_FIELD_COUNT ? 0 : cfg_accessors[field].offset;
}

// Get the default config (i.e. the NVM image)
uint8_t config_dump(void *dest) {
  if (!dest) return 1;
  memcpy(dest, NVM_ADDR_CONFIG, sizeof(config_t));
  return 0;
}

#endif
