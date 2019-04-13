#ifndef LIBSOCKET_LIBRARY_H
#define LIBSOCKET_LIBRARY_H

#include "utils.h"

#define GATEWAY "0.0.0.0"

int socket_create(int type);
int socket_create_udp();
int socket_create_tcp();
struct sockaddr_in* socket_addr_init(int family, char* ip, unsigned short port);
void socket_bind(int sockfd, struct sockaddr_in* addr);
void socket_listen(int sockfd, int backlog);
int socket_accept(int sockfd, struct sockaddr_in* addr, size_t* addr_len);
void socket_connect(int sockfd, struct sockaddr_in* addr);
ssize_t socket_write(int sockfd, const void* buf, size_t buf_size);
ssize_t socket_read(int sockfd, void* buf, size_t buf_size);
ssize_t socket_sendto(int sockfd, const void* buf, size_t buf_size, struct sockaddr_in* addr);
ssize_t socket_recvfrom(int sockfd, void* buf, size_t buf_size, struct sockaddr_in* addr, size_t* addr_len);

#endif