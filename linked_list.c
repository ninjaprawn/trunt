#include <stdlib.h>
#include "linked_list.h"

linked_list list_create() {
    linked_list ll = malloc(sizeof(struct linked_list));
    ll->head = NULL;
    return ll;
}

void list_destroy(linked_list ll) {
    if (ll != NULL) {
        list_node n = ll->head;
        while (n != NULL) {
            list_node t = n;
            n = n->next;
            free(t);
        }

        ll->head = NULL;
        free(ll);
    }
}

uint32_t list_length(linked_list ll) {
    if (ll != NULL && ll->head != NULL) {
        uint32_t count = 0;
        list_node n = ll->head;
        while (n != NULL) {
            n = n->next;
            count++;
        }
        return count;
    }
    return 0;
}

list_node list_item_at_idx(linked_list ll, uint32_t idx) {
    if (ll != NULL && ll->head != NULL) {
        uint32_t i = 0;
        list_node n = ll->head;
        while (i != idx && n != NULL) {
            n = n->next;
            i++;
        }
        return n;
    }
    return NULL;
}

void list_append(linked_list ll, list_node n) {
    if (ll != NULL && ll->head != NULL) {
        list_node prev = ll->head;
        while (prev->next != NULL) {
            prev = prev->next;
        }
        prev->next = n;
    } else if (ll != NULL) {
        ll->head = n;
    }
}


list_node list_node_create(uint32_t size) {
    list_node n = calloc(1, size);
    n->next = NULL;
    return n;
}

void list_node_destroy(list_node n) {
    free(n);
}

