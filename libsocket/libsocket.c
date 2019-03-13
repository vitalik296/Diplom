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

struct sockaddr_in* socket_addr_init(int family, char* ip, unsigned short port) {
    struct sockaddr_in* addr = malloc(sizeof(struct sockaddr_in));

    bzero(&addr, sizeof(addr));

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

int socket_accept(int sockfd, struct sockaddr* client_addr, int* addr_len) {
    int client_fd;

    if ((client_fd = (unsigned int) accept(sockfd, client_addr, (socklen_t *) addr_len)) < 0) {
        throw();
    }

    return client_fd;
}

void socket_connect(int sockfd, struct sockaddr_in* addr, int addr_len) {
    if (connect(sockfd, (struct sockaddr *) addr, (socklen_t) addr_len) < 0) {
        throw();
    }
}

int socket_write(int sockfd, const void* buf, int buf_size) {
    ssize_t size;

    if ((size = write(sockfd, buf, (size_t) buf_size)) < 0) {
        throw();
    }

    return (int) size;
}

int socket_read(int sockfd, void* buf, int buf_size) {
    ssize_t size;

    if ((size = read(sockfd, buf, (size_t) buf_size)) < 0) {
        throw();
    }

    return (int) size;
}