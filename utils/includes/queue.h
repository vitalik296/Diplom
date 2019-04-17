//
// Created by Zaharchenko on 17.04.2019.
//

#ifndef UTILS_QUEUE_H
#define UTILS_QUEUE_H

#include <sys/types.h>

#include "common.h"

typedef struct queue_node_s {
    struct queue_node_s* prev_node;
    struct queue_node_s* next_node;
    void* data;
    size_t data_size;
} queue_node_t;

typedef struct {
    queue_node_t* head;
    queue_node_t* tail;
    size_t size;

    method_t* enqueue;
    method_t* dequeue;
    method_t* destroy;

} queue_t;

queue_t* queue_init();

#endif //UTILS_QUEUE_H
