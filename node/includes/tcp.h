#ifndef TCP_LIBRARY_H
#define TCP_LIBRARY_H

void tcp_response(int client_fd, void* data, size_t size, short status);
void tcp_response_message(int client_fd, char* message, short status);
void tcp_response_error(int client_fd, char* error);
void tcp_response_errno(int client_fd);

typedef void (*tcp_worker)(int client_fd, char* buffer);

struct sockaddr_in* tcp_run_server(tcp_worker worker);

#endif