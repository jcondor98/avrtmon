// AVR Temperature Monitor -- Paolo Lucchesi
// AVR-side Configuration - Head file
#ifndef __CONFIGURATION_MODULE_H
#define __CONFIGURATION_MODULE_H
#include <stdint.h>


// Number of field present in the configuration
//FIELD-COUNT-SUBST-HERE

// Identifiers for each configuration field
enum CONFIG_FIELD_E {
//FIELD-ENUM-SUBST-HERE
};

typedef uint8_t config_field_t;

// Type definition for the entire configuration
typedef struct _config_s {
//CTYPE-SUBST-HERE
} config_t;

// Get the size of a single field
// Returns 0 if the field does not exist
uint8_t config_get_size(config_field_t field);


// AVR-side stuff
#ifdef AVR

// Get the value of a configuration field
// The variable used to hold that value must be passed by the user
// Returns 0 on success, 1 otherwise
uint8_t config_get(config_field_t field_id, void *dest);

// Set the value of a configuration field
// Returns 0 on success, 1 otherwise
uint8_t config_set(config_field_t field_id, const void *value);

// Fetch a configuration data structure (e.g. from a NVM)
// 'src' is the address/offset in the NVM of the config data structure to fetch
uint8_t config_fetch(void);

// Save a configuration data structure living in memory to the NVM
uint8_t config_save(void);

// Same as 'config_save', but apply changes only to a single field
// 'dest' is still the base address of the config data structure in the NVM,
// any field offset and displacement will be computed by the function
// Returns 0 on success, 1 on error
uint8_t config_save_field(config_field_t field);


// Host-side stuff
#else

// Get a string representing a config field (by config field id)
// Returns a pointer to a string relative to the field id, or NULL on failure
const char *config_field_str(config_field_t field);

// Get a config field id given its id
// On success, 0 is returned and the config field value is written to 'dest'
// On failure, 1 is returned and 'dest' is not touched
int config_field_id(const char *desc, config_field_t *dest);

#endif  // AVR


// The functions below shall be used only for testing
#ifdef TEST

// Get the offset of a single field
// Returns 0 if the field does not exist
uint8_t config_get_offset(config_field_t field);

// Get the default config (i.e. the NVM image)
uint8_t config_dump(void *dest);

#endif  // TEST

#endif  // __CONFIGURATION_MODULE_H
