#include <stdlib.h>
#include <memory.h>

#include "list.h"


list_node_t* list_node_create(void* data, size_t data_size) {
    list_node_t* node = malloc(sizeof(list_node_t));

    node->data_size = data_size;
    node->prev_node = node->next_node = NULL;

    node->data = malloc(node->data_size);
    memcpy(node->data, data, node->data_size);

    return node;
}

void list_node_destroy(list_node_t* node) {
    node->prev_node = node->next_node = NULL;
    free(node->data);
    free(node);
}

void list_insert(list_t* list, void* data, size_t data_size, int position) {

    if (position < 0)
        return;

    position = (int) ((position > list->size) ? list->size : position);

    list_node_t* new_node = list_node_create(data, data_size);

    if (list->size == 0) {
        list->head = list->tail = new_node;
        list->size++;
        return;
    }else if(position == list->size){
        list->tail->next_node = new_node;
        new_node->prev_node = list->tail;
        list->tail = new_node;
    }else if(position == 0){
        new_node->next_node = list->head;
        list->head->prev_node = new_node;
        list->head = new_node;
    }
    else {
        list_node_t* tmp;

        if(position <= list->size/2) {
            tmp = list->head;
            for (int i = 0; i < position; i++)
                tmp = tmp->next_node;
        }
        else{
            tmp = list->tail;
            for (int i = (int) list->size; i >= position; i--)
                tmp = tmp->prev_node;
        }

        new_node->next_node = tmp->next_node;
        new_node->prev_node = tmp;
        tmp->next_node->prev_node = new_node;
        tmp->next_node = new_node;

    }
    list->size++;
}

list_node_t* list_delete(list_t* list, int position) {

    if ((list->size == 0)||(position < 0))
        return NULL;

    position = (int) ((position > list->size) ? list->size : position);

    list_node_t* del_node;

    if(position <= list->size/2) {
        del_node = list->head;
        for (int i = 0; i < position; i++)
            del_node = del_node->next_node;
    }
    else{
        del_node = list->tail;
        for (int i = (int) list->size; i > position; i--)
            del_node = del_node->prev_node;
    }

    if (list->size == 1) {
        list->tail = list->head = NULL;
    } else if(position == 0){
        del_node->next_node->prev_node = NULL;
        list->head = del_node->next_node;
    }else if(position == list->size){
        del_node->prev_node->next_node = NULL;
        list->tail = del_node->prev_node;
    }else{
        del_node->prev_node->next_node = del_node->next_node;
        list->tail = list->tail->prev_node;
        del_node->prev_node = NULL;
    }

    list->size--;

    return del_node;

}

void list_destroy(list_t* list) {
    while (list->size > 0)
        list_node_destroy(list_delete(list,0));

    free(list);
}

list_t* list_init() {
    list_t* list = malloc(sizeof(list_t));
    list->head = list->tail = NULL;
    list->size = 0;
    return list;
}
