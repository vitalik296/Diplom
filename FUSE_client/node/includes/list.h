//
// Created by Zaharchenko on 25.04.2019.
//

#ifndef UTILS_LISTS_H
#define UTILS_LISTS_H

#include <sys/types.h>

#include "common.h"

typedef struct list_node_s{
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

list_node_t* list_node_create(void* data, size_t data_size);
void list_node_destroy(list_node_t* node);
void list_insert(list_t* list, void* data, size_t data_size, int position);
list_node_t* list_delete(list_t* list, int position);
void list_destroy(list_t* list);
list_t* list_init();

#endif //UTILS_LISTS_H
