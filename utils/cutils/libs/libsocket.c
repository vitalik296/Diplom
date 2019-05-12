#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <strings.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "libsocket.h"

int socket_create(int type) {
    int sockfd;

    if ((sockfd = socket(PF_INET, type, 0)) < 0) {
        throw();
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
        throw();
    }

    return sockfd;
}

int socket_create_udp() {
    return socket_create(SOCK_DGRAM);
}

int socket_create_tcp() {
    return socket_create(SOCK_STREAM);
}

int socket_timeout(int sockfd, uint32_t sec, uint32_t usec) {
    struct timeval tv = { sec, usec };
    return setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

struct sockaddr_in* socket_addr_init(int family, char* ip, unsigned short port) {
    struct sockaddr_in* addr = malloc(sizeof(struct sockaddr_in));

    addr->sin_family = (sa_family_t) family;
    addr->sin_addr.s_addr = inet_addr(ip);
    addr->sin_port = htons(port);

    return addr;
}

void socket_bind(int sockfd, struct sockaddr_in* addr) {
    if (bind(sockfd, (struct sockaddr*) addr, sizeof(struct sockaddr_in))) {
        throw();
    }
}

void socket_listen(int sockfd, int backlog) {
    if (listen(sockfd, backlog) < 0) {
        throw();
    }
}

int socket_accept(int sockfd, struct sockaddr_in* addr, size_t* addr_len) {
    int client_fd;

    if ((client_fd = accept(sockfd, (struct sockaddr*) addr, (socklen_t*) addr_len)) < 0) {
        throw();
    }

    return client_fd;
}

void socket_connect(int sockfd, struct sockaddr_in* addr) {
    if (connect(sockfd, (struct sockaddr*) addr, sizeof(struct sockaddr)) < 0) {
        throw();
    }
}

ssize_t socket_write(int sockfd, const void* buf, size_t buf_size) {
    ssize_t size;

    if ((size = write(sockfd, buf, buf_size)) < 0) {
        throw();
    }

    return size;
}

ssize_t socket_read(int sockfd, void* buf, size_t buf_size) {
    ssize_t size;

    if ((size = read(sockfd, buf, buf_size)) < 0) {
        throw();
    }

    return size;
}

ssize_t socket_sendto(int sockfd, const void* buf, size_t buf_size, struct sockaddr_in* addr) {
    ssize_t size;

    if ((size = sendto(sockfd, buf, buf_size, 0, (const struct sockaddr*) addr, sizeof(struct sockaddr))) < 0) {
        throw();
    }

    return size;
}

ssize_t socket_recvfrom(int sockfd, void* buf, size_t buf_size, struct sockaddr_in* addr, size_t* addr_len) {
    ssize_t size;

    if ((size = recvfrom(sockfd, buf, buf_size, 0, (struct sockaddr*) addr, (socklen_t*) addr_len)) < 0) {
        throw();
    }

    return size;
}