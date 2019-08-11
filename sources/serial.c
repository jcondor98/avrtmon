// avrtmon
// Serial communication layer - Source file
// Paolo Lucchesi - Fri 09 Aug 2019 07:35:01 PM CEST
//#include <avr/io.h>
#include <string.h>
#include "serial.h"

// RX variables
volatile uint8_t *rx_buffer;
volatile uint8_t rx_received;
volatile uint8_t rx_size;
volatile uint8_t rx_locked;
volatile uint8_t rx_starving;

// TX variables
#define TX_BUFFER_SIZE 64
volatile uint8_t tx_buffer[TX_BUFFER_SIZE]; // TODO: Resize to packet size
volatile uint8_t tx_to_transmit;
volatile uint8_t tx_transmitted;
volatile uint8_t tx_locked;
volatile uint8_t tx_starving;


// Initialize the USART
void serial_init(void) {
	// Set baud rate
	UBRR0H = (uint8_t) (UBRR_VALUE >> 8);
	UBRR0L = (uint8_t) UBRR_VALUE;

	// Set the communication frame width (8 bits for us)
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	// Enable TX and RX (i.e. duplex communication)
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);

    // Enable global interrupts
	sei();
}


// Send data stored in a buffer
// The data will be copied into another buffer, so it can be reused immediately
// Returns:
//   0 -> Success, data has been loaded and the first byte was already sent
//   1 -> TX is locked, but data has been loaded anyway
//   2 -> The data of the previous call has not been sent entirely yet (no
//        action done in that case, try later)
//   3 -> Inconsistent arguments were passed
uint8_t serial_send(const void *buf, uint8_t size) {
  if (!buf || !size || size > TX_BUFFER_SIZE)
    return 3;
  if (tx_transmitted && tx_transmitted != tx_to_transmit)
    return 2;
  if (tx_locked)
    return 1;

  memcpy(tx_buffer, buf, size);

  // Actually send the data -- Send the first byte with busy waiting, the others
  // will be sent by the TX ISR
  while (! (UCSR0A & (1 << UDRE0)))
      ;
  UDR0 = c;

  return 0;
}

// Receive data and store it into a buffer
void serial_recv(volatile void *buf, uint8_t size) {
  if (!buf) return;
  serial_rx_reset();
  rx_buffer = buf;
}

// Return the bytes received after the last call to 'serial_rx_reset()'
uint8_t serial_rx_available() {
  return rx_received;
}

// Return the bytes received after the last call to 'serial_tx_reset()'
uint8_t serial_tx_sent() {
  return tx_transmitted;
}

// Reset indexes for receiving data from the serial
void serial_rx_reset() {
  rx_buffer = NULL;
  rx_received = 0;
}


// Reset indexes for transmitting data with the serial
void serial_tx_reset() {
  tx_to_transmit = 0;
  tx_transmitted = 0;
}

// Returns 1 if *x is locked, 0 if it is not locked
uint8_t serial_rx_islocked() { return rx_locked; }
uint8_t serial_tx_islocked() { return tx_locked; }

// Lock and Unlock TX and RX activities
void serial_rx_lock() { rx_locked = 1; }
void serial_tx_lock() { tx_locked = 1; }
void serial_rx_unlock() { rx_locked = 0; }
void serial_tx_unlock() { tx_locked = 0; }

// RX Interrupt Service Routine
ISR(USART0_RX_vect) {
  if (rx_locked)
    rx_starving = 1;
  else if (rx_received < rx_size)
    rx_buffer[rx_received++] = UDR0;
}


// TX Interrupt Service Routine
ISR(USART0_TX_vect) {
  if (tx_locked)
    tx_starving = 1;
  else if (tx_transmitted < tx_to_transmit)
    UDR0 = tx_buffer[tx_transmitted++];
}
