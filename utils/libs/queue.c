//
// Created by Zaharchenko on 24.03.2019.
//

#include <stdlib.h>
#include <string.h>

#include "queue.h"

void queue_node_destroy(queue_node_t* node) {
    node->prev_node = node->next_node = NULL;
    free(node->data);
    free(node);
}

void enqueue(queue_t* queue, void* data, size_t data_size) {
    list_insert(queue, data, data_size, 0);
}

queue_node_t* dequeue(queue_t* queue) {
    return list_delete(queue, (int) queue->size);
}

void queue_destroy(queue_t* queue) {
    list_destroy(queue);
}

queue_t* queue_init() {
    return list_init();
}