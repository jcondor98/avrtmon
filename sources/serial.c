// avrtmon
// Serial communication layer - Source file
// Paolo Lucchesi - Fri 09 Aug 2019 07:35:01 PM CEST
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include "serial.h"

// RX variables
volatile uint8_t *rx_buffer = NULL;
volatile uint8_t rx_received = 0;
volatile uint8_t rx_size = 0;
volatile uint8_t rx_locked = 0;
volatile uint8_t rx_starving = 0;

// TX variables
#define TX_BUFFER_SIZE 64
volatile uint8_t tx_buffer[TX_BUFFER_SIZE]; // TODO: Resize to packet size
volatile uint8_t tx_to_transmit = 0;
volatile uint8_t tx_transmitted = 0;
volatile uint8_t tx_locked = 0;
volatile uint8_t tx_starving = 0;


// Initialize the USART
void serial_init(void) {
	// Set baud rate
	UBRR0H = (uint8_t) (UBRR_VALUE >> 8);
	UBRR0L = (uint8_t) UBRR_VALUE;

	// Set the communication frame width (8 bits for us)
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	// Enable TX and RX (i.e. duplex communication) and RX and UDRE interrupts
    // TODO: UDRIE or UDRE?
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); // | (1 << UDRIE0);

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
  // Test against inconsistent parameters
  if (!buf || !size || size > TX_BUFFER_SIZE)
    return 3;

  // Test against unfinished transmission
  if (tx_transmitted && tx_transmitted != tx_to_transmit)
    return 2;

  // Setup for transmission
  tx_to_transmit = size;
  tx_transmitted = 1; // First byte is sent manually with busy-waiting
  for (int i=0; i < size; ++i)
    tx_buffer[i] = ((uint8_t*) buf)[i];

  // Test against TX lock
  if (tx_locked)
    return 1;

  // Actually send the data -- Send the first byte with busy waiting, the others
  // will be sent by the TX ISR
  // TODO: Could deadlock happen here? If yes, how to prevent it?
  while (! (UCSR0A & (1 << UDRE0)))
    ;
  UDR0 = tx_buffer[0];

  // Enable UDR Emptied Interrupt
  UCSR0B |= (1 << UDRIE0);

  return 0;
}

// Receive data and store it into a buffer
void serial_recv(volatile void *buf, uint8_t size) {
  if (!buf) return;
  uint8_t starving = rx_starving;

  // Setup for receiving
  serial_rx_reset();
  rx_buffer = buf;
  rx_size = size;

  // Handle possibly starving byte in UDR0
  // TODO: Consider dropping the waiting data
  if (starving)
    rx_buffer[rx_received++] = UDR0;
}

// Return the bytes received after the last call to 'serial_rx_reset()'
uint8_t serial_rx_available(void) {
  return rx_received;
}

// Return the bytes received after the last call to 'serial_tx_reset()'
uint8_t serial_tx_sent(void) {
  return tx_transmitted;
}

// Reset indexes for receiving data from the serial
void serial_rx_reset(void) {
  rx_buffer = NULL;
  rx_received = 0;
}


// Reset indexes for transmitting data with the serial
void serial_tx_reset(void) {
  tx_to_transmit = 0;
  tx_transmitted = 0;
}

// Returns 1 if *x is locked, 0 if it is not locked
uint8_t serial_rx_islocked(void) { return rx_locked; }
uint8_t serial_tx_islocked(void) { return tx_locked; }

// Lock and Unlock TX and RX activities
void serial_rx_lock(void) { rx_locked = 1; }
void serial_tx_lock(void) { tx_locked = 1; }
void serial_rx_unlock(void) { rx_locked = 0; }
void serial_tx_unlock(void) { tx_locked = 0; }

// RX Interrupt Service Routine
ISR(USART0_RX_vect) {
  if (rx_locked || rx_received >= rx_size)
    rx_starving = 1;
  else
    rx_buffer[rx_received++] = UDR0;
}


// TX Interrupt Service Routine
ISR(USART0_UDRE_vect) {
  UCSR0B &= ~(1 << UDRIE0); // Disable UDRE Interrupt
  if (tx_locked)
    tx_starving = 1;
  else if (tx_transmitted < tx_to_transmit) {
    UDR0 = tx_buffer[tx_transmitted++];
    UCSR0B |= (1 << UDRIE0); // Enable UDRE Interrupt again
  }
}
