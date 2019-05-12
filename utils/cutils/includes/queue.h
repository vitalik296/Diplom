//
// Created by Zaharchenko on 17.04.2019.
//

#ifndef UTILS_QUEUE_H
#define UTILS_QUEUE_H

#include <sys/types.h>

#include "common.h"
#include "list.h"

typedef list_node_t queue_node_t;
typedef list_t queue_t;

queue_t* queue_init();
void queue_destroy(queue_t* queue);
void enqueue(queue_t* queue, void* data, size_t data_size);
queue_node_t* dequeue(queue_t* queue);

void queue_node_destroy(queue_node_t* node);

#endif //UTILS_QUEUE_H
