// avrtmon
// Ring Buffer data structure - Source file
// Paolo Lucchesi - Unit test
#include "test_framework.h"
#include "ringbuffer.h"

#define BUF_SIZE 8


int main(int argc, const char *argv[]) {
  printf("avrtmon - Circular Buffer Unit Test\n\n");
  uint8_t ret;
  uint8_t _dest, *dest = &_dest;

  // Initialize the buffer
  uint8_t buf_raw[BUF_SIZE];
  ringbuffer_t _buf, *buf = &_buf;

  ret = ringbuffer_new(buf, buf_raw, BUF_SIZE);
  test_expr(ret == 0, "New buffer should be created succesfully");
  uint8_t used = ringbuffer_used(buf);
  test_expr(ringbuffer_used(buf) == 0,
      "New buffer should be empty (currently there are %hhu items)", used);
  test_expr(ringbuffer_pop(buf, dest) != 0, "Pop on an empty buffer should fail");
  putchar('\n');


  // Fill the buffer
  for (uint8_t i=0; i < BUF_SIZE; ++i) {
    ret = ringbuffer_push(buf, i);
    test_expr(ret == 0, "Push operation should be successful");
    uint8_t used = ringbuffer_used(buf);
    test_expr(used == i+1, "Size should be incremented (current is %hhu)", used);
  }
  test_expr(ringbuffer_isfull(buf) == 1, "The buffer should be considered as full");
  test_expr(ringbuffer_isempty(buf) == 0, "The buffer should not be considered as empty");

  // Try to add another element
  ret = ringbuffer_push(buf, BUF_SIZE);
  test_expr(ret != 0, "Push operation should fail when the buffer is full");
  test_expr(ringbuffer_used(buf) == BUF_SIZE, "Size should not be incremented");
  test_expr(ringbuffer_isfull(buf) == 1, "The buffer should still be considered as full");
  test_expr(ringbuffer_isempty(buf) == 0, "The buffer should still not be considered as empty");
  putchar('\n');


  // Empty a half of the buffer
  for (uint8_t i=0; i < BUF_SIZE/2; ++i) {
    ret = ringbuffer_pop(buf, dest);
    test_expr(ret == 0, "Pop operation should be successful");
    uint8_t used = ringbuffer_used(buf);
    test_expr(used == BUF_SIZE-i-1,
        "Size should be decremented (current is %hhu)", used);
    test_expr(*dest == i, "The correct element (%hhu) should be returned", *dest);
  }
  putchar('\n');


  // Try to refill the buffer
  for (uint8_t i=BUF_SIZE; i < BUF_SIZE + BUF_SIZE/2; ++i) {
    ret = ringbuffer_push(buf, i);
    test_expr(ret == 0, "Push operation should be successful");
  }
  test_expr(ringbuffer_used(buf) == BUF_SIZE, "Buffer should have BUF_SIZE items");
  test_expr(ringbuffer_isfull(buf) == 1, "The buffer should be considered as full");
  putchar('\n');


  // Empty the buffer completely
  for (uint8_t i=BUF_SIZE/2; i < BUF_SIZE + BUF_SIZE/2; ++i) {
    ret = ringbuffer_pop(buf, dest);
    test_expr(ret == 0, "Pop operation should be successful");
    test_expr(*dest == i, "The correct element (%hhu) should be returned", *dest);
  }
  test_expr(ringbuffer_isempty(buf) == 1, "The buffer should be considered as empty");

  test_summary();
}
