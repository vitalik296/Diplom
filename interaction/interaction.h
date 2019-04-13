#ifndef INTERACTION_LIBRARY_H
#define INTERACTION_LIBRARY_H

typedef enum {
    _create_udp_connection_,
    _shutdown_node_
} operations;

typedef enum { FAILURE, SUCCESS } status;

typedef struct {
    operations operation;
    size_t size;
    void* data;
} request_t;

typedef struct {
    status status;
    size_t size;
    void* data;
} response_t;

request_t* request_init(operations operation, void* data, size_t size);
void request_free(request_t* request);
response_t* response_init(void* data, size_t size, short status);
void response_free(response_t* response);

#endif