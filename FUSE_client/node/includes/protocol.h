//
// Created by Zaharchenko on 17.04.2019.
//

#ifndef UTILS_PROTOCOL_H
#define UTILS_PROTOCOL_H

#include <sys/types.h>

#include "queue.h"
#include "logger.h"
#include "libsocket.h"

#define MAX_DATA_SIZE 50
#define HEADER_SIZE 15
#define RESPONSE_BUFFER_SIZE 5
#define TTR 16
#define THREAD_COUNT 4
#define MIDDLE_WRITE_IP "0.0.0.0"
#define MIDDLE_WRITE_PORT 8888

typedef enum { SUCCESS, FAILURE } status;

typedef struct {
    int32_t index;
    status status;
} answer_t;

void transmit_data(void* data, size_t size, int fd, queue_t* queue, void* hash_table);
status secure_sendto(int server_fd, const void* data, size_t size, struct sockaddr_in* client_addr);
void* w_worker(queue_t* queue);


#endif //UTILS_PROTOCOL_H
