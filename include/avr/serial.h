// avrtmon
// Serial communication layer - Source file
// Paolo Lucchesi - Fri 09 Aug 2019 07:35:56 PM CEST
// NOTE: Beware the way you send data from the host to the AVR board; if you
// send a long string, bigger than the buffer, expect data to be dropped. If you
// want to send a lot of data in one stroke, you have to wait between each byte.
// This is because most (or every, dunno) AVR boards do not have a real,
// reasonably large buffer for the usart communication, in fact, they have a
// duplex single byte register (i.e. UDR0[RX] and UDR0[TX])
#ifndef __SERIAL_LAYER_H
#define __SERIAL_LAYER_H
#include <stdint.h>

// USART Parameters
#define BAUD_RATE 57600
#define UBRR_VALUE (F_CPU / 16 / BAUD_RATE - 1)

// Transmission buffer size - Default is 128
#ifndef TX_BUFFER_SIZE
#define TX_BUFFER_SIZE 128
#endif


// Initialize the USART
void serial_init(void);

// Receive at most 'size' bytes of data, storing them into an external buffer
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

// Return 1 if *x is ongoing, 0 otherwise
uint8_t serial_rx_ongoing(void);
uint8_t serial_tx_ongoing(void);

// Reset the RX and TX state of the serial interface
void serial_rx_reset(void);
void serial_tx_reset(void);

#endif    // __SERIAL_LAYER_H
