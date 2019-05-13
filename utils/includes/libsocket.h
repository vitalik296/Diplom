#ifndef UTILS_LIBSOCKET_H
#define UTILS_LIBSOCKET_H

#include <stdio.h>
#include <arpa/inet.h>

#include "logger.h"

#define GATEWAY "0.0.0.0"

typedef struct sockaddr_in address_t;
typedef struct sockaddr sockaddr_t;

typedef struct {
    int fd;
    logger_t* logger;
} socket_t;

socket_t* socket_init(logger_t* logger, int type);
socket_t* socket_init_udp(logger_t* logger);
socket_t* socket_init_tcp(logger_t* logger);
int socket_timeout(socket_t* self, time_t sec, suseconds_t usec);
address_t* socket_addr_init(sa_family_t family, char* ip, uint16_t port);
int socket_bind(socket_t* self, address_t* address);
int socket_listen(socket_t* self, int backlog);
socket_t* socket_accept(socket_t* self);
int socket_connect(socket_t* self, address_t* address);
ssize_t socket_write(socket_t* self, const void* buffer, size_t buffer_size);
ssize_t socket_read(socket_t* self, void* buffer, size_t buffer_size);
ssize_t socket_sendto(socket_t* self, const void* buffer, size_t buffer_size, address_t* address, socklen_t address_size);
ssize_t socket_recvfrom(socket_t* self, void* buffer, size_t buffer_size, address_t* address, socklen_t* address_size);
void socket_log(socket_t* self, char* msg, log_type type);
void socket_free(socket_t* self);

#endif //UTILS_LIBSOCKET_H