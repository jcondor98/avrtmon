// avrtmon
// Serial communication layer - Source file
// Paolo Lucchesi - Fri 09 Aug 2019 07:35:56 PM CEST
#ifndef __SERIAL_LAYER_H
#define __SERIAL_LAYER_H
#include <stdint.h>

// USART Parameters
#define BAUD_RATE 19200
#define UBRR_VALUE (F_CPU / 16 / BAUD_RATE - 1)


// Initialize the USART
void serial_init(void);

// Receive data and store it into a buffer
void serial_rx(volatile void *buf, uint8_t size);

// Send data stored in a buffer
// The data will be copied into another buffer, so it can be reused immediately
// Returns:
//   0 -> Success, data has been loaded and the first byte was already sent
//   1 -> Inconsistent arguments were passed
//   2 -> The data of the previous call has not been sent entirely yet (no
//        action done in that case, try later)
uint8_t serial_tx(const void *buf, uint8_t size);

// Return the number of bytes received after the last call to 'serial_rx_reset'
uint8_t serial_rx_available(void);

// Return the number of bytes received after the last call to 'serial_tx_reset'
uint8_t serial_tx_sent(void);

// Reset indexes for receiving data from the serial
void serial_rx_reset(void);

// Reset indexes for transmitting data with the serial
void serial_tx_reset(void);

// Return 1 if *x is ongoing, 0 otherwise
uint8_t serial_rx_ongoing(void);
uint8_t serial_tx_ongoing(void);

#endif    // __SERIAL_LAYER_H
