#ifndef LIBSOCKET_LIBRARY_H
#define LIBSOCKET_LIBRARY_H

#include "utils.h"

int socket_create(int type);
int socket_create_udp();
int socket_create_tcp();
struct sockaddr_in* socket_addr_init(int family, char* ip, unsigned short port);
void socket_bind(int sockfd, struct sockaddr_in* addr);
void socket_listen(int sockfd, int backlog);
int socket_accept(int sockfd, struct sockaddr* client_addr, int* addr_len);
void socket_connect(int sockfd, struct sockaddr_in* addr, int addr_len);
int socket_write(int sockfd, const void* buf, int buf_size);
int socket_read(int sockfd, void* buf, int buf_size);

#endif