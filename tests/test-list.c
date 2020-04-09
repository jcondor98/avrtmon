// AVR Temperature Monitor -- Paolo Lucchesi
// Linked List - Test Unit
#include <string.h>
#include "test_framework.h"
#include "list.h"

#define TEST_ITEMS_MAX 3  // Avoid long iterations for repetitive tests


int main(int argc, const char *argv[]) {
  printf("avrtmon - Linked List Unit Test\n");
  list_t *l, _l2, *l2 = &_l2;  // l2 is almost always accessed directly, for testing
  int ret, *ret_p;

  // Initialize test items
  int values[TEST_ITEMS_MAX];
  for (int i=0; i < TEST_ITEMS_MAX; ++i)
    values[i] = i;


  // Start testing
  printf("\nTesting list_new()\n");
  l = list_new();
  *l2 = (list_t) { 0 };  // We know that a new list has got all 0/NULL fields
  test_expr(memcmp(l, l2, sizeof(list_t)) == 0, "The new list should be empty");


  printf("\nTesting list_add_tail()\n");
  ret = list_add_tail(l, &values[0]);
  test_expr(ret == 0, "list_add_tail() call should be successful");
  test_expr(l->size == 1, "List should contain one element now");
  test_expr(l->head == l->tail && l->head != NULL,
      "List head and tail should equal with one element");

  for (int i=1; i < TEST_ITEMS_MAX; ++i) {
    ret = list_add_tail(l, (void*) (values + i));
    test_expr(ret == 0, "list_add_tail() call should be successful");
    test_expr(l->size == i+1,
        "List size should effectively equal the number of elements");
  }


  printf("\nTesting list_get() with bad parameters\n");
  ret = list_get(NULL, 0, (void*) &ret_p);
  test_expr(ret != 0, "list_get() call should be unsuccessful with NULL list");
  ret = list_get(l, 0, NULL);
  test_expr(ret != 0,
      "list_get() call should be unsuccessful with NULL destination pointer");
  ret = list_get(l, TEST_ITEMS_MAX, (void*) &ret_p);
  test_expr(ret != 0, "list_get() call should be unsuccessful when fetching"
      " the element of index %d", TEST_ITEMS_MAX);

  printf("\nTesting list_get()\n");
  for (int i=0; i < TEST_ITEMS_MAX; ++i) {
    ret = list_get(l, i, (void*) &ret_p);
    test_expr(ret == 0, "list_get(%d) call should be successful", i);
    test_expr(ret_p == values + i,
        "list_get(%d) should give the correct value of the list node", i);
  }


  printf("\nTesting list_remove() with bad parameters\n");
  ret = list_remove(NULL, 0, NULL);
  test_expr(ret != 0, "list_remove() call should be unsuccessful with NULL list");
  ret = list_remove(l, TEST_ITEMS_MAX, NULL);
  test_expr(ret != 0, "list_remove() call should be unsuccessful when removing"
      " the element of index %d", TEST_ITEMS_MAX);

  printf("\nTesting list_remove()\n");
  for (int i=0; i < TEST_ITEMS_MAX; ++i) {
    ret = list_remove(l, 0, (void*) &ret_p);
    test_expr(ret == 0, "list_remove() call should be successful");
    test_expr(ret_p == values + i,
        "list_remove() should give the correct value of the list node");
    test_expr(l->size == TEST_ITEMS_MAX - i - 1, "List size should be consistent");
  }


  // Refill the list with TEST_ITEMS_MAX elements
  for (int i=0; i < TEST_ITEMS_MAX; ++i)
    list_add_tail(l, values + i);


  printf("\nTesting list_add_at()\n");
  ret = list_add_at(l, 1, &values[0]); // 2nd item should have value 0
  test_expr(ret == 0, "list_add_at(1) call should be successful");
  list_get(l, 1, (void*) &ret_p);
  test_expr(ret_p == &values[0], "List item value should be consistent");
  test_expr(l->size == TEST_ITEMS_MAX + 1, "List size should be consistent");

  printf("\nTesting list_remove() with a middle element\n");
  ret = list_remove(l, 1, (void*) &ret_p);
  test_expr(ret == 0, "list_remove(1) call should be successful");
  test_expr(ret_p == &values[0], "The right element should be removed");
  test_expr(l->size == TEST_ITEMS_MAX, "List size should be consistent");


  list_delete(l, NULL);
  test_summary();
  return 0;
}
