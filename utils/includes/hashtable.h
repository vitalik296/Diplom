//
// Created by Zaharchenko on 20.04.2019.
//

#ifndef UTILS_HASHTABLE_H
#define UTILS_HASHTABLE_H

#include "list.h"

#define DEFAULT_CAPACITY 150
#define DEFAULT_LOAD_FACTOR 2.0f

typedef struct{
    void* key;
    int key_size;
    int value;
}hashtable_node_t;


typedef struct{
    list_t** table;
    int size;
    int capacity;
    float load_factor;
}hashtable_t;


hashtable_t* hashtable_init(int capacity, float load_factor);
void hashtable_destroy(hashtable_t* hashtable);

void hashtable_add(hashtable_t* hashtable, void* key, int key_size, int value);
hashtable_node_t* hashtable_remove(hashtable_t* hashtable, void* key, int key_size);
hashtable_node_t* hashtable_get(hashtable_t* hashtable, void* key, int key_size);

void hashtable_rehash(hashtable_t* hashtable);
void hashtable_print(hashtable_t* hashtable);


#endif //UTILS_HASHTABLE_H
