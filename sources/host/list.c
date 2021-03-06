// AVR Temperature Monitor -- Paolo Lucchesi
// Linked list - Source file
#include <stdlib.h>
#include <string.h>
#include "list.h"


// [AUX] Create a new list node
static inline list_node_t *list_node_new(void *value, list_node_t *next);

// [AUX] Get a list node by index
static inline list_node_t *list_node_get(list_t *l, size_t index);


// Create a new linked list
// Return a pointer to the new, empty list on success, NULL otherwise
list_t *list_new(void) {
  list_t *l = malloc(sizeof(list_t));
  if (!l) return NULL;
  memset(l, 0, sizeof(list_t));
  return l;
}

// Delete a linked list, with all their nodes and relative values
// 'item_destroyer', if not NULL, will be called on every item of the list.
void list_delete(list_t *l, void (*item_destroyer)(void*)) {
  if (!l) return;
  list_node_t *n = l->head;
  while (n) {
    list_node_t *next = n->next;
    if (n->value && item_destroyer)
      item_destroyer(n->value);
    free(n);
    n = next;
  }
  free(l);
}

// Return the size of a list
size_t list_size(list_t *l) { return l ? l->size : 0; }


// Add an item to the head of the list
int list_add_head(list_t *l, void *value) {
  list_node_t *n = list_node_new(value, l->head);
  if (!n) return 1;

  l->head = n;
  if (l->size == 0) l->tail = n;

  l->size++;
  return 0;
}


// Add an item to the tail of the list
int list_add_tail(list_t *l, void *value) {
  list_node_t *n = list_node_new(value, NULL);
  if (!n) return 1;

  if (l->size != 0) l->tail->next = n;
  else l->head = n;
  l->tail = n;

  l->size++;
  return 0;
}


// Add an item as the 'index'-th element
int list_add_at(list_t *l, size_t index, void *value) {
  if (!l || index > l->size) return 1;
  if (index == 0)       return list_add_head(l, value);
  if (index == l->size) return list_add_tail(l, value);

  list_node_t *n_prec = list_node_get(l, index - 1);
  list_node_t *n = list_node_new(value, n_prec->next);
  if (!n) return 1;

  n_prec->next = n;
  l->size++;
  return 0;
}


// Get the value of a list node by index -- Value is returned as a pointer
// On success, 0 is returned and the requested value is copied into 'value'
// On failure, 1 is returned and 'value' is not touched
int list_get(list_t *l, size_t index, void **value) {
  if (!l || index >= l->size || !value) return 1;
  list_node_t *n = list_node_get(l, index);
  *value = n->value;
  return 0;
}


// Get the first item value for which 'predicate' returns true (i.e. !0)
// If no match is found, NULL is returned
void *list_find(list_t *l, int (*predicate)(void*)) {
  if (!l || !predicate) return NULL;
  for (list_node_t *n=l->head; n; n = n->next)
    if (predicate(n->value)) return n->value;
  return NULL;
}


// Remove the 'index'-th element from the list
// On success, 0 is returned and the removed value is copied into 'value', if
// specified. On failure, 1 is returned and 'value' is not touched
// 'value' can be NULL; if so, the memory area pointed by the value of the list
// node will be deallocated
int list_remove(list_t *l, size_t index, void **value) {
  if (!l || index >= l->size) return 1;
  list_node_t *n, *n_prec;

  switch (l->size) {
    case 1:
      n = l->head;
      l->head = NULL;
      l->tail = NULL;
      break;

    case 2:
      if (index == 0) {
        n = l->head;
        l->head = l->tail;
      }
      else {
        n = l->tail;
        l->tail = l->head;
      }
      break;

    default:
      if (index == 0) {  // i.e. remove head
        n = l->head;
        l->head = n->next;
      }

      else {
        n_prec = list_node_get(l, index - 1);
        n = n_prec->next;
        n_prec->next = n->next;
        if (index == l->size - 1) l->tail = n_prec;  // i.e. remove tail
      }

      break;
  }

  // Take care of the value of the list node to be removed
  if (value) *value = n->value;
  else free(n->value);

  free(n);
  l->size--;
  return 0;
}


// Concat two lists, i.e. attach 'l2' to the tail of 'l1'
// 'l2' will be destroyed and must NOT be used after a call to this function
void list_concat(list_t *l1, list_t *l2) {
  if (!l1 || !l2) return;
  if (l2->size != 0) {
    if (l1->size == 0) memcpy(l1, l2, sizeof(list_t));
    else {
      l1->tail->next = l2->head;
      l1->tail = l2->tail;
      l1->size += l2->size;
    }
  }
  free(l2);
}



// Create a new list iterator from a list
list_iterator_t list_iterator_new(list_t *l) {
  return l ? l->head : NULL;
}

// Get the value of the node currently pointed by an iterator
void *list_iterator_getvalue(list_iterator_t it) {
  return it ? it->value : NULL;
}

// Shift iterator -- Functional
list_iterator_t list_iterator_next(list_iterator_t it) {
  return it ? it->next : NULL;
}



// [AUX] Create a new list node
static inline list_node_t *list_node_new(void *value, list_node_t *next) {
  list_node_t *n = malloc(sizeof(list_node_t));
  if (!n) return NULL;
  *n = (list_node_t) { .value = value, .next = next };
  return n;
}

// [AUX] Get a list node by index
static inline list_node_t *list_node_get(list_t *l, size_t index) {
  list_node_t *n = l->head;
  for (int i=0; i < index; ++i)
    n = n->next;
  return n;
}
