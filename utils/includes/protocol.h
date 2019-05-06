#ifndef UTILS_PROTOCOL_H
#define UTILS_PROTOCOL_H

#include <sys/types.h>

#include "queue.h"
#include "logger.h"
#include "hashtable.h"
#include "libsocket.h"

#define MAX_DATA_SIZE 50
#define PACKAGE_SIZE MAX_DATA_SIZE+MAX_DATA_SIZE
#define TTR 16

typedef enum { FAILURE, SUCCESS } status;

typedef struct {
    pthread_t* threads;
    size_t number;
    queue_t* queue;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    address_t* address;
    logger_t* logger;
} protocol_t;

/* // HOW USE
 *
 * logger_t* logger = logger_init(NULL);
 *
 * protocol_t* protocol = protocol_init(THREAD_COUNT, MIDDLE_IP, MIDDLE_WRITE_PORT, logger);
 *
 *
 * // client
 * protocol_start(protocol, protocol_transmitter);
 * protocol_transmit(protocol, fd, buffer, size);
 *
 * // server
 * protocol_start(protocol, my_worker);
 * protocol_receiver_start(protocol);
 *
 *
 * protocol_stop(protocol);
 *
 * protocol_destroy(protocol);
 *
 * free(logger);
 */

// protocol //
protocol_t* protocol_init(size_t number, char* ip, ushort port, logger_t* logger);
void protocol_start(protocol_t* protocol, void* worker);
void protocol_stop(protocol_t* protocol);
void protocol_destroy(protocol_t* protocol);

// queue //
void protocol_enqueue_package(protocol_t* protocol, void* package, uint32_t size);
queue_node_t* protocol_dequeue_node(protocol_t* protocol);

// transmitter //
void protocol_transmit(protocol_t* protocol, uint32_t fd, void* data, uint32_t size, hashtable_t* hashtable);
void protocol_transmitter(protocol_t* protocol);

// receiver //
void protocol_receiver_start(protocol_t* protocol);

#endif //UTILS_PROTOCOL_H