// AVR Temperature Monitor -- Paolo Lucchesi
// Ring Buffer data structure - Test Unit
#include "test_framework.h"
#include "ringbuffer.h"

#define BUF_SIZE 8


int main(int argc, const char *argv[]) {
  printf("avrtmon - Multi-Threaded Circular Buffer Unit Test\n\n");
  unsigned char ret, dest[1];
  size_t used;

  // Initialize the buffer
  ringbuffer_t *buf = ringbuffer_new(BUF_SIZE);
  test_expr(buf != NULL, "New buffer should be created succesfully");
  used = ringbuffer_used(buf);
  test_expr(used == 0, "New buffer should be empty (currently there are %zu items)", used);
  test_expr(ringbuffer_pop(buf, dest) != 0, "Pop on an empty buffer should fail");
  putchar('\n');


  // Fill the buffer
  for (size_t i=0; i < BUF_SIZE; ++i) {
    ret = ringbuffer_push(buf, i);
    test_expr(ret == 0, "Push operation should be successful");
    used = ringbuffer_used(buf);
    test_expr(used == i+1, "Size should be incremented (current is %zu)", used);
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
  for (size_t i=0; i < BUF_SIZE/2; ++i) {
    ret = ringbuffer_pop(buf, dest);
    test_expr(ret == 0, "Pop operation should be successful");
    size_t used = ringbuffer_used(buf);
    test_expr(used == BUF_SIZE-i-1,
        "Size should be decremented (current is %zu)", used);
    test_expr(*dest == i, "The correct element (%hhu) should be returned", *dest);
  }
  putchar('\n');


  // Try to refill the buffer
  for (size_t i=BUF_SIZE; i < BUF_SIZE + BUF_SIZE/2; ++i) {
    ret = ringbuffer_push(buf, i);
    test_expr(ret == 0, "Push operation should be successful");
  }
  test_expr(ringbuffer_used(buf) == BUF_SIZE, "Buffer should have BUF_SIZE items");
  test_expr(ringbuffer_isfull(buf) == 1, "The buffer should be considered as full");
  putchar('\n');


  // Empty the buffer completely
  for (size_t i=BUF_SIZE/2; i < BUF_SIZE + BUF_SIZE/2; ++i) {
    ret = ringbuffer_pop(buf, dest);
    test_expr(ret == 0, "Pop operation should be successful");
    test_expr(*dest == i, "The correct element (%hhu) should be returned", *dest);
  }
  test_expr(ringbuffer_isempty(buf) == 1, "The buffer should be considered as empty");

  test_summary();
}
