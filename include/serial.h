// avrtmon
// Serial communication layer - Source file
// Paolo Lucchesi - Fri 09 Aug 2019 07:35:56 PM CEST
#ifndef __SERIAL_LAYER_H
#define __SERIAL_LAYER_H
#include <stdint.h>


// Initialize the USART
void serial_init(void);

// Send data stored in a buffer
// The data will be copied into another buffer, so it can be reused immediately
// Returns:
//   0 -> Success, data has been loaded and the first byte was already sent
//   1 -> TX is locked, but data has been loaded anyway
//   2 -> The data of the previous call has not been sent entirely yet (no
//        action done in that case, try later)
//   3 -> Inconsistent arguments were passed
uint8_t serial_send(const void *buf, uint8_t size);

// Receive data and store it into a buffer
void serial_recv(volatile void *buf, uint8_t size);

// Return the bytes received after the last call to 'serial_rx_reset()'
uint8_t serial_rx_available();

// Return the bytes received after the last call to 'serial_tx_reset()'
uint8_t serial_tx_sent();

// Reset indexes for receiving data from the serial
void serial_rx_reset();

// Reset indexes for transmitting data with the serial
void serial_tx_reset();

// Returns 1 if *x is locked, 0 if it is not locked
uint8_t serial_rx_islocked();
uint8_t serial_tx_islocked();

// Lock and Unlock TX and RX activities
void serial_rx_lock();
void serial_tx_lock();
void serial_rx_unlock();
void serial_tx_unlock();

#endif    // __SERIAL_LAYER_H

