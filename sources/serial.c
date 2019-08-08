#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "serial.h"
#include "packet.h"


// USART Parameters
#define BAUD_RATE 19600
#define UBRR_VALUE (F_CPU / 16 / BAUD_RATE - 1)

// Variables for serial RX
// TODO: Determine wether receiving data should be locked
volatile packet_t packet_rx;
volatile uint8_t *packet_rx_raw = &packet_rx;
volatile uint8_t rx_idx = 0;
//volatile uint8_t rx_size = 0;

// Variables for serial TX
volatile packet_t packet_tx;
volatile uint8_t *packet_tx_raw = &packet_tx;
volatile uint8_t tx_idx = 0;
volatile uint8_t tx_size = 0;

// ID of the last acknowledged packet
volatile uint8_t last_acked_id = 0x0F;
#define last_acked_id_inc() do { \
  last_acked_id = (last_acked_id + 1) % PACKET_ID_MAX_VAL; \
} while (0);

// Interrupt Service Routine for reading
ISR(USART0_RX_vect) {
  //if (!rx_size) return;
  packet_rx_raw[rx_idx] = UDR0;

  switch (rx_idx) {

    // 1st byte of header -- Contains type and id of the packet
    case 0:
      switch (packet_rx->type) {
        case PACKET_TYPE_ACK:
          last_acked_id_inc();
          return;
        case PACKET_TYPE_ERR:
          // TODO: Resend the previously sent packet
          return;
        default:
          ++rx_idx;
          return;
      } break;

    // 2nd byte of header -- Contains size and header parity
    case 1:
      if (packet_header_parity(packet_rx) == 0) {
        rx_size = packet_rx->size;
        ++rx_idx;
      }
      else {
        // TODO: Send error packet
      }
      break;

    // Generic data/CRC byte of the packet
    default:
      if (++rx_idx == rx_size)
        rx_idx = rx_size = 0;
      break;
  }
}

// Interrupt Service Routine for writing
ISR(USART0_TX_vect) {
}


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


// Write a single character
// Used to push only the first byte of data
void serial_putchar(uint8_t c) {
	while (! (UCSR0A & (1 << UDRE0)))
		;
	UDR0 = c;
}
