// avrtmon
// Test Unit on data structures metadata et similia - Host side
// Paolo Lucchesi - Wed 18 Dec 2019 12:49:34 AM CET
#include <stdio.h>
#include <stddef.h>

#include "packet.h"
#include "command.h"


// For now, only packet and command structures are tested
// TODO: Test other stuff
int main(int argc, const char *argv[]) {
  puts("Metadata for data types test unit\n");
  // Test packet_t
  printf("Metadata for packet_t:\n"
      "sizeof packet_t -> %u\n"
      "[ Ignoring address for bitfields ]\n"
      "offsetof .data -> %u\n\n",
      sizeof(packet_t), offsetof(packet_t, data));

  // Test command_t
  // Note that the data type target architecture depends on the 'AVR' macro
  printf("Metadata for command_payload_t\n"
      "command_payload_t ends with a variable length member (.arg)\n"
      "sizeof command_payload_t -> %zu\n"
      "offsetof arg -> %zu\n\n",
      sizeof(command_payload_t), offsetof(command_payload_t, arg));

  return 0;
}
