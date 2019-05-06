#ifndef UDP_LIBRARY_H
#define UDP_LIBRARY_H

void udp_response(int server_fd, struct sockaddr_in* client_addr, void* data, size_t size, short status);
void udp_response_message(int server_fd, struct sockaddr_in* client_addr, char* message, short status);
void udp_response_error(int server_fd, struct sockaddr_in* client_addr, char* error);
void udp_response_errno(int server_fd, struct sockaddr_in* client_addr);

typedef int (*udp_worker)(int server_fd, struct sockaddr_in* client_addr, char* buffer);

int udp_create_connection(struct sockaddr_in* addr);
struct sockaddr_in* udp_run_server(udp_worker worker);

#endif