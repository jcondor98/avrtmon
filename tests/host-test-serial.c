// AVR Temperature Monitor -- Paolo Lucchesi
// Serial module test unit - Host-side
#include <stdio.h>
#include <stdlib.h>

#include "host/serial.h"

#define DEV_PATH "/dev/ttyACM0"
#define READY_MSG "Ready to receive\n"
#define BUF_SIZE 48


int main(int argc, const char *argv[]) {
  serial_context_t *ctx = serial_open(DEV_PATH);
  if (!ctx) {
    fprintf(stderr, "Unable to initialize serial context\n");
    exit(EXIT_FAILURE);
  }

  unsigned i;

  /*
  // Receive ready message
  for (i=0; i < sizeof(READY_MSG) - 1; )
    if (serial_rx_getchar(ctx, buf_rx + i)) ++i;
  buf_rx[i] = '\0';
  printf("[AVR] %s\n", buf_rx);
  */

  /* Version 2 */

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


  /* Version 1 */

  /*
  char buf_rx[RX_BUF_SIZE];

  // Send/receive the alphabet
  i = 0;
  for (char c='a'; c < 'z' + 1; ++c, ++i) {
    write(ctx->dev_fd, &c, 1);
    while (!serial_rx_getchar(ctx, buf_rx + i))
      ;
    //printf("Received: %c\n", buf_rx[i]);
  }

  serial_close(ctx);

  buf_rx[i] = '\0';
  printf("\n[AVR] %s\n\n", buf_rx);
  */
}
