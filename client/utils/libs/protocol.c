#include "protocol.h"

typedef enum { FAILURE, SUCCESS } status;

// protocol //

protocol_t* protocol_init(size_t number, char* ip, uint16_t port, logger_t* logger) {
    protocol_t* protocol = malloc(sizeof(protocol_t));

    protocol->threads = malloc(number * sizeof(pthread_t));
    protocol->number = number;
    protocol->queue = queue_init();
    pthread_mutex_init(&protocol->mutex, NULL);
    pthread_cond_init(&protocol->cond, NULL);
    protocol->address = socket_addr_init(AF_INET, ip, port);
    protocol->logger = logger;

    return protocol;
}

void protocol_start(protocol_t* protocol, void* worker) {
    for (int i = 0; i < protocol->number; i++) {
        if (pthread_create(&protocol->threads[i], NULL, worker, protocol) != 0) {
            logger_write(protocol->logger, "protocol_start", ERROR);
        }
    }
}

void protocol_stop(protocol_t* protocol) {
    for (int i = 0; i < protocol->number; i++) {
        pthread_cancel(protocol->threads[i]);
    }
}

void protocol_free(protocol_t* protocol) {
    free(protocol->threads);
    queue_free(protocol->queue);
    pthread_mutex_destroy(&protocol->mutex);
    pthread_cond_destroy(&protocol->cond);
    free(protocol->address);
    free(protocol);
}

// queue //

void protocol_enqueue_package(protocol_t* protocol, void* package, size_t size) {
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

void pack(void* package, uint32_t fd, void* data, uint32_t size, uint32_t number) { // Remove uint8_t latest param
    size_t offset = 0;

    memcpy(package + offset, &fd, sizeof(fd));
    offset += sizeof(fd);

    memcpy(package + offset, &number, sizeof(number));
    offset += sizeof(number);

//    memcpy(package + offset, &latest, sizeof(latest));
//    offset += sizeof(latest);

    memcpy(package + offset, &size, sizeof(size));
    offset += sizeof(size);

    memcpy(package + offset, data, size);
    offset = PACKAGE_SIZE - sizeof(uint16_t);

    uint16_t checksum = hash_function(package, offset);
    memcpy(package + offset, &checksum, sizeof(checksum));
}

void protocol_transmit(protocol_t* protocol, uint32_t fd, void* data, uint32_t size, hashtable_t* hashtable) {
    void* package = malloc(PACKAGE_SIZE);

    hashtable_node_t* node = hashtable_get(hashtable, &fd, sizeof(uint32_t));

    uint32_t length;
    uint32_t offset = 0;
    uint32_t pack_num = (uint32_t) (node != NULL ? node->value : 0);

    do {
        length = size > MAX_DATA_SIZE ? MAX_DATA_SIZE : size;

        memset(package, 0, PACKAGE_SIZE);
        pack(package, fd, data + offset, length, pack_num);
        protocol_enqueue_package(protocol, package, PACKAGE_SIZE);

        offset += length;
        size -= length;
        pack_num++;

    } while (size > 0);

    hashtable_set(hashtable, &fd, sizeof(int), pack_num);

    free(package);
}

status protocol_secure_sendto(protocol_t* protocol, socket_t* server, queue_node_t* node) {
    for (int ttr = 0; ttr < TTR; ttr++) {
        if (socket_sendto(server, node->data, node->data_size, protocol->address, sizeof(address_t)) > 0) {
            return SUCCESS;
        }
    }

    return FAILURE;
}

void protocol_transmitter(protocol_t* protocol) { // similar to transmit in transmitter.c
    status status, answer;
    queue_node_t* node;

    socket_t* server = socket_init_udp(protocol->logger);
    socket_timeout(server, 10, 0);

    while (1) {
        status = FAILURE;

        node = protocol_dequeue_node(protocol);

        for (int ttr = 0; ttr < TTR; ttr++) {

            if (protocol_secure_sendto(protocol, server, node) == FAILURE) {
                socket_log(server, "Can not sendto client", WARNING);
                continue;
            }

            if (socket_recvfrom(server, &answer, sizeof(answer), NULL, NULL) < 0) {
                continue;
            }

            if (answer == FAILURE) {
                socket_log(server, "Package status is incorrect", WARNING);
                continue;
            }

            status = SUCCESS;

            break;
        }

        if (status == FAILURE) {
            socket_log(server, "Can not sendto or recvfrom package", WARNING);
        }

        queue_node_free(node);
    }
}

// receiver //

void send_status(socket_t* server, status status, address_t* address, socklen_t address_size) {
    socket_sendto(server, &status, sizeof(status), address, address_size);
}

void protocol_receiver_start(protocol_t* protocol) {
    socket_t* server = socket_init_udp(protocol->logger);
    socket_bind(server, protocol->address);

    address_t client_address;
    socklen_t address_size;

    void* package = malloc(PACKAGE_SIZE);

    while (1) {
        memset(package, 0, sizeof(package));

        if (socket_recvfrom(server, package, PACKAGE_SIZE, &client_address, &address_size) < 0) {
            send_status(server, FAILURE, &client_address, address_size);
            continue;
        }

        uint16_t checksum;
        size_t offset = PACKAGE_SIZE - sizeof(uint16_t);
        memcpy(&checksum, package + offset, sizeof(checksum));

        if (hash_function(package, offset) == checksum) {
            protocol_enqueue_package(protocol, package, PACKAGE_SIZE);
            send_status(server, SUCCESS, &client_address, address_size);
        } else {
            send_status(server, FAILURE, &client_address, address_size);
            logger_write(protocol->logger, "Checksum is incorrect", WARNING);
        }
    }

    free(package);
}
