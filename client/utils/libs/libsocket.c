#include "libsocket.h"

socket_t* socket_init(logger_t* logger, int type) {
    int socket_fd;

    if ((socket_fd = socket(PF_INET, type, 0)) < 0) {
        logger_write(logger, "socket_init", ERROR);
        return NULL;
    }

    socket_t* self = malloc(sizeof(socket_t));

    self->fd = socket_fd;
    self->logger = logger;

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &(int) { 1 }, sizeof(int)) < 0) {
        socket_log(self, "setsockopt", ERROR);
    }

    return self;
}

socket_t* socket_init_udp(logger_t* logger) {
    return socket_init(logger, SOCK_DGRAM);
}

socket_t* socket_init_tcp(logger_t* logger) {
    return socket_init(logger, SOCK_STREAM);
}

int socket_timeout(socket_t* self, time_t sec, suseconds_t usec) {
    struct timeval tv = { sec, usec };

    if (setsockopt(self->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        socket_log(self, "socket_timeout", ERROR);
        return -1;
    }

    return 0;
}

address_t* socket_addr_init(sa_family_t family, char* ip, uint16_t port) {
    address_t* address = malloc(sizeof(address_t));

    address->sin_family = family;
    address->sin_addr.s_addr = inet_addr(ip);
    address->sin_port = htons(port);

    return address;
}

int socket_bind(socket_t* self, address_t* address) {
    if (bind(self->fd, (sockaddr_t*) address, sizeof(address_t))) {
        socket_log(self, "socket_bind", ERROR);
        return -1;
    }

    return 0;
}

int socket_listen(socket_t* self, int backlog) {
    if (listen(self->fd, backlog) < 0) {
        socket_log(self, "socket_listen", ERROR);
        return -1;
    }

    return 0;
}

socket_t* socket_accept(socket_t* self) {
    int client_fd;

    address_t address;
    socklen_t address_size = sizeof(address);

    if ((client_fd = accept(self->fd, (sockaddr_t*) &address, &address_size)) < 0) {
        socket_log(self, "socket_accept", ERROR);
        return NULL;
    }

    socket_t* client = malloc(sizeof(socket_t));

    client->fd = client_fd;
    client->logger = self->logger;

    return client;
}

int socket_connect(socket_t* self, address_t* address) {
    if (connect(self->fd, (sockaddr_t*) address, sizeof(address_t)) < 0) {
        socket_log(self, "socket_connect", ERROR);
        return -1;
    }

    return 0;
}

ssize_t socket_write(socket_t* self, const void* buffer, size_t buffer_size) {
    ssize_t size;

    if ((size = write(self->fd, buffer, buffer_size)) < 0) {
        socket_log(self, "socket_write", ERROR);
    }

    return size;
}

ssize_t socket_read(socket_t* self, void* buffer, size_t buffer_size) {
    ssize_t size;

    if ((size = read(self->fd, buffer, buffer_size)) < 0) {
        socket_log(self, "socket_read", ERROR);
    }

    return size;
}

ssize_t socket_sendto(socket_t* self, const void* buffer, size_t buffer_size, address_t* address, socklen_t address_size) {
    ssize_t size;

    if ((size = sendto(self->fd, buffer, buffer_size, 0, (sockaddr_t*) address, address_size)) < 0) {
        socket_log(self, "socket_sendto", ERROR);
    }

    return size;
}

ssize_t socket_recvfrom(socket_t* self, void* buffer, size_t buffer_size, address_t* address, socklen_t* address_size) {
    ssize_t size;

    if ((size = recvfrom(self->fd, buffer, buffer_size, 0, (sockaddr_t*) address, address_size)) < 0) {
        socket_log(self, "socket_recvfrom", ERROR);
    }

    return size;
}

void socket_log(socket_t* self, char* msg, log_type type) {
    char text[strlen(msg) + 64];
    sprintf(text, "%s; fd: %d", msg, self->fd);
    logger_write(self->logger, text, type);
}

void socket_free(socket_t* self) {
    close(self->fd);
    free(self);
}
