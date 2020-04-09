// AVR Temperature Monitor -- Paolo Lucchesi
// Serial communication layer - Source file
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stddef.h>
#include "serial.h"
#include "ringbuffer.h"

// Polling time for blocking RX in milliseconds
#define RX_BLOCKING_POLLING_TIME_MS 5

// RX variables
static volatile uint8_t rx_buffer_raw[RX_BUFFER_SIZE];
static volatile ringbuffer_t rx_buffer[1];
static uint8_t rx_ongoing;

// TX variables
static uint8_t tx_buffer[TX_BUFFER_SIZE];
static uint8_t tx_to_transmit;
static volatile uint8_t tx_transmitted;
static volatile uint8_t tx_ongoing;


// [AUX] Enable and disable RX and TX interrupts
static inline void rx_sei(void) { UCSR0B |=   1 << RXCIE0 ; }
static inline void rx_cli(void) { UCSR0B &= ~(1 << RXCIE0); }
static inline void tx_sei(void) { UCSR0B |=   1 << TXCIE0 ; }
static inline void tx_cli(void) { UCSR0B &= ~(1 << TXCIE0); }


// Initialize the USART
void serial_init(void) {
  // Iniialize RX circular buffer - Ignore errors (there won't be any)
  ringbuffer_new((ringbuffer_t*) rx_buffer,
      (uint8_t*) rx_buffer_raw, RX_BUFFER_SIZE);

  // Set baud rate
  UBRR0H = (uint8_t) (UBRR_VALUE >> 8);
  UBRR0L = (uint8_t) UBRR_VALUE;

  // Set the communication frame width (8 bits for us)
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

  // Enable TX and RX (i.e. duplex communication)
  // RX and UDRE interrupts are enabled dynamically by 'serial_*x'
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);

  rx_sei(); // Start receiving immediately
  rx_ongoing = 1;
}


// RX Interrupt Service Routine
ISR(USART0_RX_vect) {
  if (!ringbuffer_isfull((ringbuffer_t*) rx_buffer))
    ringbuffer_push((ringbuffer_t*) rx_buffer, UDR0);
}


// TX Interrupt Service Routine
ISR(USART0_TX_vect) {
  ++tx_transmitted;
  if (tx_transmitted < tx_to_transmit)
    UDR0 = tx_buffer[tx_transmitted];
  else serial_tx_reset();
}


// Receive at most 'size' bytes of data, storing them into an external buffer
// Returns the number of bytes read
uint8_t serial_rx(void *buf, uint8_t size) {
  if (!buf) return 0;
  uint8_t n;
  for (n=0; n < size; ++n) {
    if (ringbuffer_isempty((ringbuffer_t*) rx_buffer))
      return n;
    ringbuffer_pop((ringbuffer_t*) rx_buffer, buf + n);
  }
  return n;
}

// Same as 'serial_rx', blocking
// Return the number of bytes read
uint8_t serial_rx_blocking(void *buf, uint8_t size) {
  if (!buf) return 0;
  uint8_t n;
  for (uint8_t n=0; n < size; ++n) {
    if (!ringbuffer_isempty((ringbuffer_t*) rx_buffer))
      ringbuffer_pop((ringbuffer_t*) rx_buffer, buf + n);
    else _delay_ms(RX_BLOCKING_POLLING_TIME_MS); // Wait a while
  }
  return n;
}


// Send data stored in a buffer
// The data will be copied into another buffer, so it can be reused immediately
// Returns 0 on success, 1 on failure
// The function blocks until the previous transmission, if any, is completed,
// and returns immediately, not waiting for all the data to be already sent
uint8_t serial_tx(const void *buf, uint8_t size) {
  // Test against inconsistent parameters
  if (!buf || !size || size > TX_BUFFER_SIZE) return 1;

  // Block until the previous transmission is finished
  while (tx_ongoing) ;

  // Setup for transmission
  tx_ongoing = 1;
  tx_to_transmit = size;
  tx_transmitted = 0;
  for (int i=0; i < size; ++i)
    tx_buffer[i] = ((uint8_t*) buf)[i];

  // Enable TX interrupt, and send the first byte;
  // the others will be sent by the TX ISR
  tx_sei();
  UDR0 = tx_buffer[0];

  return 0;
}


// Return the number of bytes received after the last call to 'serial_rx'
uint8_t serial_rx_available(void) {
  return ringbuffer_used((ringbuffer_t*) rx_buffer);
}

// Return the number of bytes received after the last call to 'serial_tx'
uint8_t serial_tx_sent(void) { return tx_transmitted; }

// Return 1 if *x is ongoing, 0 otherwise
uint8_t serial_rx_ongoing(void) { return rx_ongoing; }
uint8_t serial_tx_ongoing(void) { return tx_ongoing; }


// Reset indexes for receiving data from the serial
void serial_rx_reset(void) {
  rx_cli();
  ringbuffer_flush((ringbuffer_t*) rx_buffer);
  rx_sei();
}

// Reset indexes for transmitting data with the serial
void serial_tx_reset(void) {
  tx_cli();
  tx_to_transmit = 0;
  tx_transmitted = 0;
  tx_ongoing = 0;
}
