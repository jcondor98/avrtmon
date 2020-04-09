// AVR Temperature Monitor -- Paolo Lucchesi
// Debug and error handling facilities - Host side
#ifndef __DEBUG_H
#define __DEBUG_H

#ifndef DEBUG
#define DEBUG 0
#endif

// If we just define DEBUG without a value, it is redefined to 1 (if non-zero)
#if defined(DEBUG) && DEBUG != 0
#undef DEBUG
#define DEBUG 1
#endif

// Use as 'debug action' or 'debug { many actions }'
#define debug if (DEBUG)

#endif  // __DEBUG_H
