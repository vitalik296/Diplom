#ifndef UTILS_TRANSMITTER_H
#define UTILS_TRANSMITTER_H

#include <pthread.h>

#include "queue.h"
#include "logger.h"
#include "hashtable.h"
#include "libsocket.h"

#define TTR 16
#define MAX_DATA_SIZE 50
#define PACKAGE_SIZE (16 + MAX_DATA_SIZE)

typedef struct {
    pthread_t* threads;
    size_t number;
    queue_t* queue;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    logger_t* logger;
} sender_t;

sender_t* sender_init(size_t number, logger_t* logger);
void sender_start(sender_t* sender);
void sender_enqueue(sender_t* sender, char* ip, uint16_t port, void* package, size_t package_size);
void sender_stop(sender_t* sender);
void sender_free(sender_t* sender);

#endif //UTILS_TRANSMITTER_H
