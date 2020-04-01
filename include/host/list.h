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
  list_node_t *head;
  list_node_t *tail;
  size_t size;
} list_t;


// Create a new linked list
// Return a pointer to the new, empty list on success, NULL otherwise
list_t *list_new(void);

// Delete a linked list, with all their nodes and relative values
// 'item_destroyer', if not NULL, will be called on every item of the list.
void list_delete(list_t *l, void (*item_destroyer)(void*));

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

// Get the first item value for which 'predicate' returns true (i.e. !0)
// If no match is found, NULL is returned
void *list_find(list_t *l, int (*predicate)(void*));

// Remove the 'index'-th element from the list
// On success, 0 is returned and the removed value is copied into 'value', if
// specified. On failure, 1 is returned and 'value' is not touched
// 'value' can be NULL; if so, the memory area pointed by the value of the list
// node will be deallocated
int list_remove(list_t *l, size_t index, void **value);
#define list_remove_head(l,v) list_remove(l,0,v)
#define list_remove_tail(l,v) list_remove(l,list_size(l),v)

// Concat two lists, i.e. attach 'l2' to the tail of 'l1'
// 'l2' will be destroyed and must NOT be used after a call to this function
void list_concat(list_t *l1, list_t *l2);


// Treat a list as a queue
typedef list_t queue_t;
#define queue_size(q) list_size(q)
#define queue_push(q,val) list_add_head(q,val)
#define queue_pop(q,dst) list_remove_tail(q,dst)


#endif    // __LINKED_LIST_H
