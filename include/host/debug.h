// avrtmon
// Debug facilities
// Paolo Lucchesi - Sat 30 Nov 2019 02:35:28 AM CET
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

#define debug if (DEBUG)

#endif    // __DEBUG_H
