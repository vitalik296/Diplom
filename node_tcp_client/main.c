#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#define PORT 12001
#define MAXLINE 1024
typedef enum {
    _create_udp_connection_,
    _shutdown_node_
} operations;

typedef enum { FAILURE, SUCCESS } status;

typedef struct {
    operations operation;
    void* data;
    size_t size;
} request_t;

typedef struct {
    void* data;
    size_t size;
    status status;
} response_t;

request_t* request_init(operations operation, void* data, size_t size);
void request_free(request_t* request);
response_t* response_init(void* data, size_t size, short status);
void response_free(response_t* response);

request_t* request_init(operations operation, void* data, size_t size) {
    request_t* request = malloc(sizeof(request_t));

    request->operation = operation;
    request->data = malloc(size * sizeof(void*));
    memcpy(request->data, data, size);
    request->size = size;

    return request;
}

void request_free(request_t* request) {
    free(request->data);
    free(request);
}

response_t* response_init(void* data, size_t size, short status) {
    response_t* response = malloc(sizeof(response_t));

    response->data = malloc(size * sizeof(void*));
    memcpy(response->data, data, size);
    response->size = size;
    response->status = status;

    return response;
}

void response_free(response_t* response) {
    free(response->data);
    free(response);
}

int main()
{
    int sockfd;
    char buffer[MAXLINE];
    char* message = "Hello Server";
    struct sockaddr_in servaddr;

    int n, len;
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket creation failed");
        exit(0);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr*)&servaddr,
                sizeof(servaddr)) < 0) {
        printf("\n Error : Connect Failed \n");
    }

    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, "Hello Server");

    request_t* request = request_init(_create_udp_connection_, "12345", 5);

    write(sockfd, request, sizeof(request_t));
    printf("Message from server: ");
    read(sockfd, buffer, sizeof(buffer));
    puts(buffer);
    close(sockfd);

    request_free(request);
}