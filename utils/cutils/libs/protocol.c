#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <pthread.h>
#include <netinet/in.h>

#include "protocol.h"

// protocol //

protocol_t* protocol_init(size_t number, char* ip, ushort port, logger_t* logger) {
    protocol_t* protocol = malloc(sizeof(protocol_t));

    protocol->threads = malloc(number * sizeof(pthread_t));
    protocol->number = number;
    protocol->queue = queue_init();
    pthread_mutex_init(&protocol->mutex, NULL);
    pthread_cond_init(&protocol->cond, NULL);
    protocol->address = socket_addr_init(AF_INET, ip, port);
    protocol->logger = logger;

    return protocol;
};

void protocol_start(protocol_t* protocol, void* worker) {
    for (int i = 0; i < protocol->number; i++) {
        if (pthread_create(&protocol->threads[i], NULL, (void* (*)(void*)) worker, protocol) != 0) {
            throw();
        }
    }
}
void protocol_stop(protocol_t* protocol) {
    for (int i = 0; i < protocol->number; i++) {
        pthread_cancel(protocol->threads[i]);
    }
}


void protocol_destroy(protocol_t* protocol) {
    free(protocol->threads);
    queue_destroy(protocol->queue);
    pthread_mutex_destroy(&protocol->mutex);
    pthread_cond_destroy(&protocol->cond);
    free(protocol->address);
    free(protocol);
}
// queue //

void protocol_enqueue_package(protocol_t* protocol, void* package, uint32_t size) {
    pthread_mutex_lock(&protocol->mutex);

    enqueue(protocol->queue, package, size);

    pthread_cond_signal(&protocol->cond);

    pthread_mutex_unlock(&protocol->mutex);
}

queue_node_t* protocol_dequeue_node(protocol_t* protocol) {
    pthread_mutex_lock(&protocol->mutex);

    while (!protocol->queue->size) {
        pthread_cond_wait(&protocol->cond, &protocol->mutex);
    }

    queue_node_t* node = dequeue(protocol->queue);

    pthread_mutex_unlock(&protocol->mutex);

    return node;
}

// transmitter //

void pack(void* package, uint32_t fd, void* data, uint32_t size, uint32_t number, uint8_t latest) {
    size_t offset = 0;
    memcpy(package + offset, &fd, sizeof(fd));

    offset += sizeof(fd);
    memcpy(package + offset, &number, sizeof(number));

    offset += sizeof(number);
    memcpy(package + offset, &latest, sizeof(latest));

    offset += sizeof(latest);
    memcpy(package + offset, &size, sizeof(size));

    offset += sizeof(size);
    memcpy(package + offset, data, size);

    offset = PACKAGE_SIZE - sizeof(uint16_t);
    uint16_t checksum = crc16(package, offset);
    memcpy(package + offset, &checksum, sizeof(checksum));
}

void protocol_transmit(protocol_t* protocol, uint32_t fd, void* data, uint32_t size, hashtable_t* hashtable) {
    void* package = malloc(PACKAGE_SIZE);

    uint32_t length = 0;
    uint32_t offset = 0;
    uint32_t pack_num = (uint32_t) hashtable_get(hashtable, &fd, sizeof(uint32_t))->value;
    uint8_t latest = 0;

    do {
        if (size > MAX_DATA_SIZE) {
            length = MAX_DATA_SIZE;
            latest = 0;
        } else {
            length = size;
            latest = 1;
        }

        memset(package, 0, PACKAGE_SIZE);
        pack(package, fd, data + offset, length, pack_num, latest);
        protocol_enqueue_package(protocol, package, PACKAGE_SIZE);

        offset += length;
        size -= length;
        pack_num++;

    } while (size > 0);

    hashtable_add(hashtable, &fd, sizeof(int), pack_num);
    free(package);
}

status protocol_secure_sendto(protocol_t* protocol, int server_fd, queue_node_t* node) {
    for (int ttr = 0; ttr < TTR; ttr++) {
        if (sendto(server_fd, node->data, node->data_size, 0, (const struct sockaddr*) protocol->address, sizeof(struct sockaddr)) < 0) {
            write_log(protocol->logger, "Can not sendto client", ERROR);
        } else {
            return SUCCESS;
        }
    }

    return FAILURE;
}

void protocol_transmitter(protocol_t* protocol) {
    status status, answer;
    queue_node_t* node;

    int server_fd = socket_create_udp();
    socket_timeout(server_fd, 10, 0);

    while (1) {
        status = FAILURE;

        node = protocol_dequeue_node(protocol);

        for (int ttr = 0; ttr < TTR; ttr++) {

            if (protocol_secure_sendto(protocol, server_fd, node) == FAILURE) {
                write_log(protocol->logger, "Can not sendto client (secure)", WARNING);
                continue;
            }

            if (recvfrom(server_fd, &answer, sizeof(answer), 0, NULL, NULL) < 0) {
                write_log(protocol->logger, "Can not recvfrom server", ERROR);
                continue;
            }

            if (answer == FAILURE) {
                write_log(protocol->logger, "Package status is incorrect", WARNING);
                continue;
            }

            status = SUCCESS;

            break;
        }

        if (status == FAILURE) {
            write_log(protocol->logger, "Can not sendto or recvfrom package", WARNING);
        }

        queue_node_destroy(node);
    }
}

// receiver //

void protocol_send_status(protocol_t* protocol, int socket_fd, status status, struct sockaddr* client) {
    if (sendto(socket_fd, &status, sizeof(status), 0, client, sizeof(struct sockaddr)) < 0) {
        write_log(protocol->logger, "Receiver: can not sendto", ERROR);
    }
}

void protocol_receiver_start(protocol_t* protocol) {
    int server_fd = socket_create_udp();
    socket_bind(server_fd, protocol->address);

    struct sockaddr client;
    socklen_t socklen;

    void* package = malloc(PACKAGE_SIZE);

    while (1) {
        memset(package, 0, sizeof(package));

        if (recvfrom(server_fd, package, PACKAGE_SIZE, 0, &client, &socklen) <= 0) {
            protocol_send_status(protocol, server_fd, FAILURE, &client);
            write_log(protocol->logger, "Can not recvfrom client", ERROR);
            continue;
        }

        uint16_t checksum;
        size_t offset = PACKAGE_SIZE - sizeof(uint16_t);
        memcpy(&checksum, package + offset, sizeof(checksum));

        if (crc16(package, offset) == checksum) {
            protocol_enqueue_package(protocol, package, PACKAGE_SIZE);
            protocol_send_status(protocol, server_fd, SUCCESS, &client);
        } else {
            protocol_send_status(protocol, server_fd, FAILURE, &client);
            write_log(protocol->logger, "Checksum is incorrect", WARNING);
        }
    }

    free(package);
}