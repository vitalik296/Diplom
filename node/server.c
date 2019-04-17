#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory.h>

#define PORT 5000
#define TCP_PORT 12001
#define BACKLOG 5
#define BUFFER_SIZE 1024

#include "udp.h"
#include "tcp.h"
#include "interaction.h"

#include "server.h"


//response_t* handler(request_t* request) {
//    switch (request->operation) {
//        case _create_udp_connection_: {
//            struct sockaddr_in* server_addr = udp_run_server();
//            response_t* server_response = response_init(server_addr, sizeof(struct sockaddr_in), SUCCESS);
//            free(server_addr);
//            return server_response;
//        }
//    }
//
//    char* error = "incorrect operation";
//    return response_init(error, strlen(error), FAILURE);
//}

//void tcp_run_worker(int client_fd) {
//    request_t* client_request = malloc(sizeof(request_t));
//
//    socket_read(client_fd, client_request, sizeof(request_t));
//
//    response_t* server_response = handler(client_request);
//
//    socket_write(client_fd, server_response, sizeof(response_t));
//
//    request_free(client_request);
//    response_free(server_response);
//
//    close(client_fd);
//}


//typedef void (*tcp_worker)(int client_fd, char* buffer);
//
//void tcp_run_worker(int server_fd, tcp_worker worker) {
//    size_t addr_len;
//    struct sockaddr_in client_addr;
//
//    char buffer[BUFFER_SIZE];
//
//    while(1) {
//        addr_len = sizeof(struct sockaddr_in);
//
//        int client_fd = socket_accept(server_fd, &client_addr, &addr_len);
//
//        pid_t pid = fork();
//
//        if (pid == 0) {
//            close(server_fd);
//
//            bzero(buffer, BUFFER_SIZE);
//
//            socket_read(client_fd, buffer, BUFFER_SIZE);
//
//            worker(client_fd, buffer);
//
//            close(client_fd);
//            exit(EXIT_SUCCESS);
//        } else if (pid < 0) {
//            response_errno(client_fd);
//        }
//
//        close(client_fd);
//    }
//}
//
//int tcp_create_connection(struct sockaddr_in* addr) {
//    int sockfd = socket_create_tcp();
//    addr = socket_addr_init(AF_INET, GATEWAY, 0);
//    socket_bind(sockfd, addr);
//    socket_listen(sockfd, BACKLOG);
//    return sockfd;
//}
//
//struct sockaddr_in* tcp_run_server(tcp_worker worker) {
//    struct sockaddr_in* server_addr = malloc(sizeof(struct sockaddr_in));
//
//    int server_fd = tcp_create_connection(server_addr);
//
//    pid_t pid = fork();
//
//    if (pid == 0) {
//        tcp_run_worker(server_fd, worker);
//        exit(EXIT_SUCCESS);
//    } else if (pid < 0) {
//        throw();
//    }
//
//    close(server_fd);
//
//    return server_addr;
//}
//
//void tcp_run_server() {
//    int server_fd = socket_create_tcp();
//
//    struct sockaddr_in* server_addr = socket_addr_init(AF_INET, GATEWAY, TCP_PORT);
//
//    socket_bind(server_fd, server_addr);
//
//    socket_listen(server_fd, BACKLOG);
//
//    size_t client_addr_len = sizeof(struct sockaddr_in);
//    struct sockaddr_in* client_addr = malloc(client_addr_len);
//
//    while(1) {
//        int client_fd = socket_accept(server_fd, client_addr, &client_addr_len);
//
//        pid_t pid = fork();
//
//        if (pid == 0) {
//            tcp_run_worker(client_fd);
//            exit(EXIT_SUCCESS);
//        } else if (pid < 0) {
//            response_errno(client_fd);
//        }
//
//        close(client_fd);
//    }
//}





//response_t* handler(request_t* request) {
//    switch (request->operation) {
//        case _create_udp_connection_: {
//            struct sockaddr_in* server_addr = udp_run_server();
//            response_t* server_response = response_init(server_addr, sizeof(struct sockaddr_in), SUCCESS);
//            free(server_addr);
//            return server_response;
//        }
//    }
//
//    char* error = "incorrect operation";
//    return response_init(error, strlen(error), FAILURE);
//}
//
//void worker(int client_fd, request_t* client_request) {
//    response_t* server_response = handler(client_request);
//
//    // server_response
////    tcp_response_message(client_fd, message, res->status);
//
//    request_free(client_request);
//    response_free(server_response);
//
//    close(client_fd);
//}
//
//void start_tcp_server() {
//    tcp_run_server();
//}