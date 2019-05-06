//
// Created by Zaharchenko on 17.04.2019.
//

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <pthread.h>
#include <netinet/in.h>

#include "protocol.h"

logger_t* logger;

pthread_mutex_t w_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t w_cond = PTHREAD_COND_INITIALIZER;




void data_decorator(void* data, uint32_t size, queue_t* queue, uint8_t is_last, uint32_t fd, uint32_t pack_num) {
    void* package = malloc(MAX_DATA_SIZE + HEADER_SIZE);

    memcpy(package, &fd, 4);
    memcpy(package + 4, &pack_num, 4);
    memcpy(package + 8, &is_last, 1); // lframe


    memcpy(package + 11, &size, 4);
    memcpy(package + 15, data, size);

    uint16_t* checksum = hash_function(package, size);
    memcpy(package + 9, checksum, 2);
    free(checksum);


    enqueue(queue, package, MAX_DATA_SIZE + HEADER_SIZE);



    free(package);

}

void transmit_data(void* data, size_t size, int fd, queue_t* queue, hashtable_t* hash_table) {
    void* package_data = malloc(MAX_DATA_SIZE);
    size_t package_size;
    size_t offset = 0;
    uint32_t pack_num = (uint32_t) hashtable_get(hash_table, &fd, sizeof(int))->value;
    uint8_t is_last = 0;

    do {
        if (size > MAX_DATA_SIZE) {
            package_size = MAX_DATA_SIZE;
            is_last = 0;
        } else {
            package_size = size;
            is_last = 1;
        }
        memcpy(package_data, data + offset, package_size);
        data_decorator(package_data, (uint32_t) package_size, queue, is_last, (uint32_t) fd, pack_num);
        size -= package_size;
        offset += package_size;
        pack_num++;
    } while (size > 0);
    hashtable_add(hash_table, &fd, sizeof(int), pack_num);
    free(package_data);
}


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

void* w_worker(queue_t* queue) {
    struct sockaddr_in* server_addr;
    int server_fd = socket_create_udp();
    server_addr = socket_addr_init(AF_INET, GATEWAY, 0);
    socket_bind(server_fd, server_addr);

    size_t addr_len;
    struct sockaddr_in* client_addr = socket_addr_init(AF_INET, MIDDLE_WRITE_IP, MIDDLE_WRITE_PORT);

    while (1) {
        // dequeue

        pthread_mutex_lock(&w_mutex);

        while (!queue->size) {
            pthread_cond_wait(&w_cond, &w_mutex);
        }

        queue_node_t* node = dequeue(queue);

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