// AVR Temperature Monitor -- Paolo Lucchesi
// Serial module test unit - Host-side
#include <stdio.h>
#include <stdlib.h>

#include "host/serial.h"

#define DEV_PATH "/dev/ttyACM0"
#define READY_MSG "Ready to receive\n"
#define BUF_SIZE 256


int main(int argc, const char *argv[]) {
  serial_context_t *ctx = serial_open(DEV_PATH);
  if (!ctx) {
    fprintf(stderr, "Unable to initialize serial context\n");
    exit(EXIT_FAILURE);
  }

  unsigned char buf_rx[BUF_SIZE];
  unsigned i;

  // Receive ready message
  for (i=0; i < sizeof(READY_MSG) - 1; )
    if (serial_rx_getchar(ctx, buf_rx + i)) ++i;
  buf_rx[i] = '\0';
  printf("[AVR] %s\n", (char*) buf_rx);

  // Wait for a user NL to begin
  getchar();

  // Transmit start byte
  static const unsigned char start_signal = '\0';
  serial_tx(ctx, &start_signal, 1);

  // Get data
  for (i=0; i <= 0xFF; ) {
    unsigned char c[1];
    if (serial_rx_getchar(ctx, c)) {
      printf(" %2hhx", c[0]);
      ++i;
    }
  }

  putchar('\n');
  serial_close(ctx);
  exit(EXIT_SUCCESS);
}
