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
volatile uint8_t rx_ongoing = 0;

// TX variables
volatile uint8_t tx_buffer[TX_BUFFER_SIZE]; // TODO: Resize to packet size
volatile uint8_t tx_to_transmit = 0;
volatile uint8_t tx_transmitted = 0;
volatile uint8_t tx_ongoing = 0;


// Initialize the USART
void serial_init(void) {
	// Set baud rate
	UBRR0H = (uint8_t) (UBRR_VALUE >> 8);
	UBRR0L = (uint8_t) UBRR_VALUE;

	// Set the communication frame width (8 bits for us)
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	// Enable TX and RX (i.e. duplex communication)
    // RX and UDRE interrupts are enabled dynamically by 'serial_*x'
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
}


// RX Interrupt Service Routine
ISR(USART0_RX_vect) {
  if (rx_received < rx_size)
    rx_buffer[rx_received++] = UDR0;
  if (rx_received >= rx_size) {  // Now 'rx_received' could be incremented
    UCSR0B &= ~(1 << RXCIE0); // RX finished, disable RXC Interrupt
    serial_rx_reset();
  }
}


// TX Interrupt Service Routine
ISR(USART0_UDRE_vect) {
  UCSR0B &= ~(1 << UDRIE0); // Disable UDRE Interrupt
  if (tx_transmitted < tx_to_transmit) {
    UDR0 = tx_buffer[tx_transmitted++];
    UCSR0B |= (1 << UDRIE0); // Enable UDRE Interrupt again
  }
  else
    serial_tx_reset();
}


// Receive at most 'size' bytes of data, storing them into an external buffer
void serial_rx(volatile void *buf, uint8_t size) {
  if (!buf) return;
  UCSR0B &= ~(1 << RXCIE0); // Disable RX Complete Interrupt

  // Setup for receiving
  rx_buffer = buf;
  rx_received = 0;
  rx_size = size;
  rx_ongoing = 1;

  // Enable RX Complete Interrupt
  UCSR0B |= (1 << RXCIE0); // Enable RX Complete Interrupt
}


// Send data stored in a buffer
// The data will be copied into another buffer, so it can be reused immediately
// Returns:
//   0 -> Success, data has been loaded and the first byte was already sent
//   1 -> Inconsistent arguments were passed
//   2 -> The data of the previous call has not been sent entirely yet (no
//        action done in that case, try later)
uint8_t serial_tx(const void *buf, uint8_t size) {
  // Test against inconsistent parameters
  if (!buf || !size || size > TX_BUFFER_SIZE)
    return 1;

  // Test against unfinished transmission
  if (tx_ongoing)
    return 2;

  // Setup for transmission
  tx_ongoing = 1;
  tx_to_transmit = size;
  for (int i=0; i < size; ++i)
    tx_buffer[i] = ((uint8_t*) buf)[i];

  // Actually send the data -- Send the first byte with busy waiting, the others
  // will be sent by the TX ISR
  // TODO: Could deadlock happen here? If yes, how to prevent it?
  while (! (UCSR0A & (1 << UDRE0)))
    ;
  UDR0 = tx_buffer[0];
  tx_transmitted = 1;

  // Enable UDR Emptied Interrupt
  UCSR0B |= (1 << UDRIE0);

  return 0;
}


// Return the number of bytes received after the last call to 'serial_rx'
uint8_t serial_rx_available(void) { return rx_received; }

// Return the number of bytes received after the last call to 'serial_tx'
uint8_t serial_tx_sent(void) { return tx_transmitted; }

// Return 1 if *x is ongoing, 0 otherwise
uint8_t serial_rx_ongoing(void) { return rx_ongoing; }
uint8_t serial_tx_ongoing(void) { return tx_ongoing; }


// Reset indexes for receiving data from the serial
void serial_rx_reset(void) {
  rx_buffer = NULL;
  rx_size = 0;
  rx_ongoing = 0;
}

// Reset indexes for transmitting data with the serial
void serial_tx_reset(void) {
  tx_to_transmit = 0;
  tx_transmitted = 0;
  tx_ongoing = 0;
}
