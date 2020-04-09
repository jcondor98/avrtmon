// AVR Temperature Monitor -- Paolo Lucchesi
// Serial communication layer - Source file
#ifndef __SERIAL_MODULE_H
#define __SERIAL_MODULE_H
#include <stdint.h>

// USART Parameters
#define BAUD_RATE 57600
#define UBRR_VALUE (F_CPU / 16 / BAUD_RATE - 1)

// Transmission buffer size - Default is 64
#ifndef TX_BUFFER_SIZE
#define TX_BUFFER_SIZE 32
#endif

// Reception buffer size - Default is 64
#ifndef RX_BUFFER_SIZE
#define RX_BUFFER_SIZE 32
#endif


// Initialize the UART
void serial_init(void);

// Read 'size' bytes, storing them into 'buf' - Non-blocking
// Returns the number of bytes read
uint8_t serial_rx(void *buf, uint8_t size);

// Same as 'serial_rx', blocking
// Return the number of bytes read
uint8_t serial_rx_blocking(void *buf, uint8_t size);

// Return a single received character
#define serial_rx_getchar(c) serial_rx(c,1)

// Send data stored in a buffer
// The data will be copied into another buffer, so it can be reused immediately
// Returns 0 on success, 1 on failure
// The function blocks until the previous transmission, if any, is completed,
// and returns immediately, not waiting for all the data to be already sent
uint8_t serial_tx(const void *buf, uint8_t size);

// Return the number of bytes received
uint8_t serial_rx_available(void);

// Return the number of bytes sent
uint8_t serial_tx_sent(void);

// Return 1 if *X is ongoing, 0 otherwise
uint8_t serial_rx_ongoing(void);
uint8_t serial_tx_ongoing(void);

// Reset the *X state of the serial interface
void serial_rx_reset(void);
void serial_tx_reset(void);

#endif    // __SERIAL_MODULE_H
