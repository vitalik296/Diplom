//
// Created by Zaharchenko on 24.03.2019.
//

#include <stdlib.h>
#include <string.h>

#include "utils.h"



queue_node_t* queue_node_create(void* data, size_t data_size)
{
    queue_node_t* node = malloc(sizeof(queue_node_t));

    node->data_size = data_size;
    node->prev_node = node->next_node = NULL;

    node->data = malloc(node->data_size);
    memcpy(node->data, data, node->data_size);

    return node;
}

void queue_node_destroy(queue_node_t* node){
    node->prev_node = node->next_node = NULL;
    free(node->data);
    free(node);
}

void enqueue(queue_t* queue, void* data, size_t data_size){
    queue_node_t* new_node = queue_node_create(data, data_size);

    queue->size++;

    if (queue->head == NULL)
    {
        queue->head = queue->tail = new_node;
        return;
    }

    new_node->next_node = queue->head;
    queue->head->prev_node = new_node;
    queue->head = new_node;

}

queue_node_t* dequeue(queue_t* queue){

    if (queue->tail == NULL)
        return NULL;

    queue->size--;

    queue_node_t* del_node = queue->tail;

    if (queue->size == 0){
        queue->tail = queue->head = NULL;
    }else {
        del_node->prev_node->next_node = NULL;
        queue->tail = queue->tail->prev_node;
        del_node->prev_node = NULL;
    }

    return del_node;

}

void destroy(queue_t* queue){
    while(queue->size > 0)
        queue_node_destroy(queue->dequeue(queue));

    free(queue);
}

queue_t* queue_init(){
    queue_t * queue = malloc(sizeof(queue_t));
    queue->head = queue->tail = NULL;
    queue->size = 0;

    queue->enqueue = (method_t*)enqueue;
    queue->dequeue = (method_t*)dequeue;
    queue->destroy = (method_t*)destroy;
    return queue;
}