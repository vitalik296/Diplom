#ifndef UTILS_LIBRARY_H
#define UTILS_LIBRARY_H

#include <stdlib.h>

// QUEUE PART
typedef void* method_t(void* this, ...);

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

// LOGGER PART
#define DEFAULT_LOG "diplom.log"

typedef enum {
    INFO, WARNING, ERROR
} log_type;

typedef struct {
    int log_d;
    method_t* write_log;
    method_t* destroy;
} logger_t;

logger_t* logger_init(char* path_name);

// COMMON PART
void throw(void);

#endif
