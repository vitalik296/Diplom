#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <errno.h>

#include "interaction.h"
#include "libsocket.h"

#include "tcp.h"

#define BUFFER_SIZE 1024
#define BACKLOG 16

// tcp functions

void tcp_response(int client_fd, void* data, size_t size, short status) {
    response_t* response = response_init(data, size, status);
    socket_write(client_fd, response, sizeof(response_t));
    response_free(response);
}

void tcp_response_message(int client_fd, char* message, short status) {
    tcp_response(client_fd, message, strlen(message), status);
}

void tcp_response_error(int client_fd, char* error) {
    tcp_response_message(client_fd, error, 0);
}

void tcp_response_errno(int client_fd) {
    tcp_response_error(client_fd, strerror(errno));
}

// tcp server initialization

void tcp_run_worker(int server_fd, tcp_worker worker) {
    size_t addr_len;
    struct sockaddr_in client_addr;

    char buffer[BUFFER_SIZE];

    while(1) {
        addr_len = sizeof(struct sockaddr_in);

        int client_fd = socket_accept(server_fd, &client_addr, &addr_len);

        pid_t pid = fork();

        if (pid == 0) {
            close(server_fd);

            bzero(buffer, BUFFER_SIZE);

            socket_read(client_fd, buffer, BUFFER_SIZE);

            worker(client_fd, buffer);

            close(client_fd);
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            tcp_response_errno(client_fd);
        }

        close(client_fd);
    }
}

int tcp_create_connection(struct sockaddr_in* addr) {
    int sockfd = socket_create_tcp();
    addr = socket_addr_init(AF_INET, GATEWAY, 0);
    socket_bind(sockfd, addr);
    socket_listen(sockfd, BACKLOG);
    return sockfd;
}

struct sockaddr_in* tcp_run_server(tcp_worker worker) {
    struct sockaddr_in* server_addr = malloc(sizeof(struct sockaddr_in));

    int server_fd = tcp_create_connection(server_addr);

    pid_t pid = fork();

    if (pid == 0) {
        tcp_run_worker(server_fd, worker);
        close(server_fd);
        exit(EXIT_SUCCESS);
    } else if (pid < 0) {
        throw();
    }

    close(server_fd);

    return server_addr;
}