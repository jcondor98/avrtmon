// avrtmon
// AVR-side Configuration - Head file
// Paolo Lucchesi - Tue 27 Aug 2019 06:58:41 PM CEST
#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H
#include <stdint.h>

// Number of field present in the configuration
//FIELD-COUNT-SUBST-HERE

// Type definition for the entire configuration
typedef struct _config_s {
//CTYPE-SUBST-HERE
} config_t;

// Identifiers for each configuration field
typedef enum CONFIG_FIELD_E {
//FIELD-ENUM-SUBST-HERE
} config_field_t;


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


// The functions below shall be used only for testing
#ifdef TEST

// Get the size and offset of a single field
uint8_t config_get_size(config_field_t field);
uint8_t config_get_offset(config_field_t field);

// Get the default config (i.e. the NVM image)
uint8_t config_dump(void *dest);

#endif


#endif    // __CONFIGURATION_H
