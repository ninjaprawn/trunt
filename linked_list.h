#ifndef _LINKED_LIST
#define _LINKED_LIST

#include <stdint.h>

typedef struct list_node {
    struct list_node* next;
}* list_node;

typedef struct linked_list {
    struct list_node* head;
}* linked_list;


linked_list list_create();
void list_destroy(linked_list ll);
uint32_t list_length(linked_list ll);
list_node list_item_at_idx(linked_list ll, uint32_t idx);
void list_append(linked_list ll, list_node n);

list_node list_node_create(uint32_t size);
void list_node_destroy(list_node n);

#endif
