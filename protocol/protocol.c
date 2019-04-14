#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <pthread.h>
#include <netinet/in.h>
#include <libsocket.h>

#include "protocol.h"

#define TTR 16

#define THREAD_COUNT 4

#define MIDDLE_WRITE_IP "0.0.0.0"
#define MIDDLE_WRITE_PORT 8888

#include "utils.h"
#include "udp.h"

logger_t* logger;
queue_t* w_queue;

pthread_mutex_t w_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t w_cond = PTHREAD_COND_INITIALIZER;

typedef enum { SUCCESS, FAILURE } status;

typedef struct {
    int32_t index;
    status status;
} answer_t;

status secure_sendto(int server_fd, const void* data, size_t size, struct sockaddr_in* client_addr) {
    for (int ttr = 0; ttr < TTR; ttr++) {
        if (socket_sendto(server_fd, data, size, client_addr) < 0) {
            logger->write_log(logger, "Can not send to client", WARNING); // socket_sendto have logger too !?
        } else {
            return 0;
        }
    }

    return -1;
}

void w_worker(queue_t* queue) {
    struct sockaddr_in server_addr;
    int server_fd = udp_create_connection(&server_addr);

    size_t addr_len;
    struct sockaddr_in* client_addr = socket_addr_init(AF_INET, MIDDLE_WRITE_IP, MIDDLE_WRITE_PORT);

    while (1) {
        todo

        // dequeue

        pthread_mutex_lock(&w_mutex);

        while (!queue->size) {
            pthread_cond_wait(&w_cond, &w_mutex);
        }

        queue_node_t* node = queue->dequeue(queue);

        pthread_mutex_unlock(&w_mutex);

        // requests

        for (int ttr = 0; ttr < TTR; ttr++) {
            if (secure_sendto(server_fd, node->data, node->data_size, client_addr) < 0) {
                logger->write_log(logger, "Can not send to client (secure)", WARNING);
                continue;
            }

            answer_t answer; // ?

            socket_recvfrom(server_fd, &answer, sizeof(answer), client_addr, &addr_len);

            if (answer.status == SUCCESS) {
                break;
            }

            if (ttr + 1 == TTR) {
                char* error = "Can not send or receive package to/from client";
                logger->write_log(logger, error, WARNING);
            }
        }
    }
}

int main() {
    logger = logger_init(NULL);
    w_queue = queue_init();

    pthread_t w_threads[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; i++) {
        if (pthread_create(&w_threads[i], NULL, &w_worker, w_queue) != 0) {
            throw();
        }
    }

    // fuse_main();

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(w_threads[i], NULL);
    }

    w_queue->destroy(w_queue);
    logger->destroy(logger);

    return EXIT_SUCCESS;
}
