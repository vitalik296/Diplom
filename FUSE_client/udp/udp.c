#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <errno.h>

#include "interaction.h"
#include "libsocket.h"

#include "udp.h"

#define BUFFER_SIZE 1024

// udp functions

void udp_response(int server_fd, struct sockaddr_in* client_addr, void* data, size_t size, short status) {
    response_t* response = response_init(data, size, status);
    socket_sendto(server_fd, response, sizeof(response_t), client_addr);
    response_free(response);
}

void udp_response_message(int server_fd, struct sockaddr_in* client_addr, char* message, short status) {
    udp_response(server_fd, client_addr, message, strlen(message), status);
}

void udp_response_error(int server_fd, struct sockaddr_in* client_addr, char* error) {
    udp_response_message(server_fd, client_addr, error, 0);
}

void udp_response_errno(int server_fd, struct sockaddr_in* client_addr) {
    udp_response_error(server_fd, client_addr, strerror(errno));
}

// udp server initialization

void udp_run_worker(int server_fd, udp_worker worker) {
    int is_alive = 1;

    size_t addr_len;
    struct sockaddr_in client_addr;

    char buffer[BUFFER_SIZE];

    while(is_alive) {
        addr_len = sizeof(struct sockaddr_in);

        bzero(buffer, BUFFER_SIZE);

        socket_recvfrom(server_fd, buffer, BUFFER_SIZE, &client_addr, &addr_len);

        is_alive = worker(server_fd, &client_addr, buffer);
    }
}

int udp_create_connection(struct sockaddr_in* addr) {
    int sockfd = socket_create_udp();
    addr = socket_addr_init(AF_INET, GATEWAY, 0);
    socket_bind(sockfd, addr);
    return sockfd;
}

struct sockaddr_in* udp_run_server(udp_worker worker) {
    struct sockaddr_in* server_addr = malloc(sizeof(struct sockaddr_in));

    int server_fd = udp_create_connection(server_addr);

    pid_t pid = fork();

    if (pid == 0) {
        udp_run_worker(server_fd, worker);
        close(server_fd);
        exit(EXIT_SUCCESS);
    } else if (pid < 0) {
        throw();
    }

    close(server_fd);

    return server_addr;
}