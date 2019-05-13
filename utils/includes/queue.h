#ifndef UTILS_QUEUE_H
#define UTILS_QUEUE_H

#include "list.h"

typedef list_node_t queue_node_t;
typedef list_t queue_t;

queue_t* queue_init();
void enqueue(queue_t* queue, void* data, size_t data_size);
queue_node_t* dequeue(queue_t* queue);
void queue_free(queue_t* queue);

queue_node_t* queue_node_init(void* data, size_t data_size);
void queue_node_free(queue_node_t* node);

#endif //UTILS_QUEUE_H