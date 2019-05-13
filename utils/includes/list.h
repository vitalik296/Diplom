#ifndef UTILS_LISTS_H
#define UTILS_LISTS_H

#include <stdlib.h>
#include <memory.h>

typedef struct list_node_s {
    struct list_node_s* prev_node;
    struct list_node_s* next_node;
    void* data;
    size_t data_size;
} list_node_t;

typedef struct {
    list_node_t* head;
    list_node_t* tail;
    size_t size;
} list_t;

list_t* list_init();
void list_insert(list_t* list, void* data, size_t data_size, int position);
list_node_t* list_delete(list_t* list, int position);
void list_free(list_t* list);

list_node_t* list_node_init(void* data, size_t data_size);
void list_node_free(list_node_t* node);

#endif //UTILS_LISTS_H