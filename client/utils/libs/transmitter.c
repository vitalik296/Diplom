#include "transmitter.h"

typedef enum { FAILURE, SUCCESS } status;

// queue item //

typedef struct queue_item_T{
//    address_t* address;
    char* ip;
    int port;
    void* package;
    size_t package_size;
} queue_item_t; // request_t

queue_item_t* queue_item_init(char* ip, uint16_t port, void* package, size_t package_size) {
    queue_item_t* item = malloc(sizeof(queue_item_t));

//    item->address = socket_addr_init(AF_INET, ip, port);
    item->ip = malloc(strlen(ip) + 1);
    strcpy(item->ip, ip);
    item->port = port;

    item->package = malloc(package_size);
    memcpy(item->package, package, package_size);

    item->package_size = package_size;

    return item;
}

void queue_item_free(queue_item_t* item) {
//    free(item->address);
    free(item->ip);
    free(item->package);
    free(item);
}

// queue //

void sender_safe_enqueue(sender_t* sender, void* data, size_t size) {
    pthread_mutex_lock(&sender->mutex);

    enqueue(sender->queue, data, size);

    pthread_cond_signal(&sender->cond);

    pthread_mutex_unlock(&sender->mutex);
}

queue_node_t* sender_safe_dequeue(sender_t* sender) {
    pthread_mutex_lock(&sender->mutex);

    while (!sender->queue->size) {
        pthread_cond_wait(&sender->cond, &sender->mutex);
    }

    queue_node_t* node = dequeue(sender->queue);

    pthread_mutex_unlock(&sender->mutex);

    return node;
}

// transmitter //

status safe_sendto(socket_t* server, queue_item_t* item) {
    address_t* address = socket_addr_init(AF_INET, item->ip, (uint16_t) item->port);

    for (int ttr = 0; ttr < TTR; ttr++) {
        if (socket_sendto(server, item->package, item->package_size, address, sizeof(address_t)) > 0) {
            free(address);
            return SUCCESS;
        }
    }

    free(address);
    return FAILURE;
}

void transmitter(sender_t* sender) {
    status status, answer;
    queue_node_t* node;
    queue_item_t* item;

    socket_t* server = socket_init_udp(sender->logger);
    socket_timeout(server, 10, 0);

    while (1) {
        status = FAILURE;

        node = sender_safe_dequeue(sender);
        item = node->data;

        for (int ttr = 0; ttr < TTR; ttr++) {
            if (safe_sendto(server, item) == FAILURE) {
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

        if (status == 0) {
            sender_safe_enqueue(sender, item, sizeof(queue_item_t));
            socket_log(server, "Can not sendto or recvfrom package", WARNING);
        } else {
//            _queue_item_free(item);
        }

        queue_node_free(node);
    }
}

// protocol //

sender_t* sender_init(size_t number, logger_t* logger) {
    sender_t* sender = malloc(sizeof(sender_t));

    sender->threads = malloc(number * sizeof(pthread_t));
    sender->number = number;
    sender->queue = queue_init();
    pthread_mutex_init(&sender->mutex, NULL);
    pthread_cond_init(&sender->cond, NULL);
    sender->logger = logger;

    return sender;
}

void sender_start(sender_t* sender) {
    for (int i = 0; i < sender->number; i++) {
        if (pthread_create(&sender->threads[i], NULL, (void*) transmitter, sender) != 0) {
            logger_write(sender->logger, "sender_start", ERROR);
        }
    }
}

void sender_enqueue(sender_t* sender, char* ip, uint16_t port, void* package, size_t package_size) {
    queue_item_t* item = queue_item_init(ip, port, package, package_size);
    sender_safe_enqueue(sender, item, sizeof(queue_item_t));
//    queue_item_free(item);
}

void sender_stop(sender_t* sender) {
    for (int i = 0; i < sender->number; i++) {
        pthread_cancel(sender->threads[i]);
    }
}

void sender_free(sender_t* sender) {
    free(sender->threads);
    queue_free(sender->queue);
    pthread_mutex_destroy(&sender->mutex);
    pthread_cond_destroy(&sender->cond);
    free(sender);
}
