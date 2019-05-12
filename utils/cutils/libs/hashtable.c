//
// Created by Zaharchenko on 20.04.2019.
//

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <stdio.h>

#include "hashtable.h"

void rehash(hashtable_t* hashtable){
    // TODO
}

hashtable_t* hashtable_init(int capacity, float load_factor) {
    hashtable_t* hashtable = malloc(sizeof(hashtable_t));

    hashtable->size = 0;
    hashtable->capacity = (capacity > 0)?capacity:DEFAULT_CAPACITY;
    hashtable->load_factor = (load_factor > 0)?load_factor:DEFAULT_LOAD_FACTOR;
    hashtable->table = malloc(hashtable->capacity*sizeof(list_t*));


    for(int i =0; i < hashtable->capacity; i++){
        hashtable->table[i] = list_init();
    }

    return hashtable;
}

void hashtable_destroy(hashtable_t* hashtable){
    for(int i =0; i < hashtable->capacity; i++){
        list_destroy(hashtable->table[i]);
    }
    free(hashtable->table);
    free(hashtable);
}


void hashtable_add(hashtable_t* hashtable, void* key, int key_size, int value){
    uint16_t hash_num = *hash_function(key, (size_t) key_size);
    unsigned int index = (unsigned int) (hash_num % hashtable->capacity);

    hashtable_node_t* new_node = malloc(sizeof(hashtable_node_t));
    new_node->key = malloc((size_t) key_size);
    memcpy(new_node->key, key, (size_t) key_size);
    new_node->value = value;

    list_t* list_to_insert = hashtable->table[index];

    if(list_to_insert->head != NULL){
        list_node_t* current_node = list_to_insert->head;
        for(int i = 0; i < list_to_insert->size; i++){
            if(!memcmp(((hashtable_node_t*)current_node->data)->key, key, (size_t) key_size)){
                ((hashtable_node_t*)current_node->data)->value = value;
                return;
            }
            current_node = current_node->next_node;
        }
    }

    list_insert(hashtable->table[index], new_node, sizeof(hashtable_node_t), 0);
    hashtable->size++;


    if((float)hashtable->size / (float)hashtable->capacity > hashtable->load_factor){
        // TODO add rehash function
    }

}


hashtable_node_t* hashtable_get(hashtable_t* hashtable, void* key, int key_size){
    uint16_t hash_value = *hash_function(key, (size_t) key_size);
    int index =(hash_value % hashtable->capacity);

    list_t* list_to_search = hashtable->table[index];
    list_node_t* getting_node = list_to_search->head;
    while(memcmp(((hashtable_node_t*)getting_node->data)->key, key, (size_t) key_size)){
        getting_node = getting_node->next_node;
        if(getting_node == NULL)
            return NULL;
    }
    return getting_node->data;
}


hashtable_node_t* hashtable_remove(hashtable_t* hashtable, void* key, int key_size){
    uint16_t hash_value = *hash_function(key, key_size);
    int index = (int) (hash_value % hashtable->capacity);

    list_t* list_to_search = hashtable->table[index];
    list_node_t* getting_node = list_to_search->head;
    int i = 0;
    while(memcmp(((hashtable_node_t*)getting_node->data)->key, key, (size_t) key_size)){
        getting_node = getting_node->next_node;
        if(getting_node == NULL)
            return NULL;
        i++;
    }

    list_delete(list_to_search, i);
;
    hashtable->size--;

    return getting_node->data;
}

void hashtable_print(hashtable_t* hashtable){
    list_t* list;
    for(int i = 0; i < hashtable->capacity; i++){
        list = hashtable->table[i];
        list_node_t* node = list->head;
        printf("Index: %d\n\t", i);
        for(int j = 0; j< list->size; j++){
            printf("{Key: %d Value: %d} -> ", *(int*)((hashtable_node_t*)node->data)->key, ((hashtable_node_t*)node->data)->value);
            node = node->next_node;
        }
        printf("\n");
    }
}

