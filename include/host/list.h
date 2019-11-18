// avrtmon
// Linked List - Head file
// Paolo Lucchesi - Wed 30 Oct 2019 01:20:38 AM CET
#ifndef __LINKED_LIST_H
#define __LINKED_LIST_H

// Type definition for a linked list node
typedef struct _list_node_s {
  void *value;
  struct _list_node_s *next;
} list_node_t;

// Type definition for a linked list
typedef struct _list_s {
  size_t size;
  list_node_t *head;
  list_node_t *tail;
} list_t;


// Create a new linked list
// Return a pointer to the new, empty list on success, NULL otherwise
list_t *list_new(void);

// Delete (i.e. destroy) a linked list, with all their nodes and relative values
void list_delete(list_t *l);

// Return the size of a list
size_t list_size(list_t *l);

// Add an item to the head of the list
int list_add_head(list_t *l, void *value);

// Add an item to the tail of the list
int list_add_tail(list_t *l, void *value);

// Add an item after the 'index'-th element
int list_add_at(list_t *l, size_t index, void *value);

// As default, 'list_add' fallbacks to 'list_add_tail'
#define list_add list_add_tail

// Get the value of a list node by index -- Value is returned as a pointer
// On success, 0 is returned and the requested value is copied into 'value'
// On failure, 1 is returned and 'value' is not touched
int list_get(list_t *l, size_t index, void **value);

// Handy list getters for head and tail
#define list_get_head(l,v) list_get(l,0,v)
#define list_get_tail(l,v) list_get(l,list_size(l),v)

// Remove the 'index'-th element from the list
// On success, 0 is returned and the removed value is copied into 'value', if
// specified. On failure, 1 is returned and 'value' is not touched
// 'value' can be NULL; if so, the memory area pointed by the value of the list
// node will be deallocated
int list_remove(list_t *l, size_t index, void **value);
#define list_remove_head(l,v) list_remove(l,0,v)
#define list_remove_tail(l,v) list_remove(l,list_size(l),v)


// Treat a list as a queue
typedef list_t queue_t;
#define queue_size(q) list_size(q)
#define queue_push(q,val) list_add_head(q,val)
#define queue_pop(q,dst) list_remove_tail(q,dst)


/* TODO: One day, when I will need, I shall implement the following:

// Type definition for a list item boolean comparator
// A function of such type must process an item and return 1 if it "match",
// 0 otherwise
typedef int (*list_comparator_f)(const void*);

// Get a list node by boolean comparator (first match is returned)
// If no match is found, NULL is returned
void *list_get_by_match(list_t *l, const void *key, list_comparator_f cmp);

*/

#endif    // __LINKED_LIST_H
