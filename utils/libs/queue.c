#include "queue.h"

/* queue */

queue_t* queue_init() {
    return list_init();
}

void queue_node_destroy(queue_node_t* node) {
    node->prev_node = node->next_node = NULL;
    free(node->data);
    free(node);
}

void enqueue(queue_t* queue, void* data, size_t data_size) {
    list_insert(queue, data, data_size, (int) queue->size);
}

queue_node_t* dequeue(queue_t* queue) {
    return list_delete(queue, 0);
}

void queue_free(queue_t* queue) {
    list_free(queue);
}

/* node */

queue_node_t* queue_node_init(void* data, size_t data_size) {
    return list_node_init(data, data_size);
}

void queue_node_free(queue_node_t* node) {
    list_node_free(node);
}
