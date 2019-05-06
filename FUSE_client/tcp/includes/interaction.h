#ifndef INTERACTION_LIBRARY_H
#define INTERACTION_LIBRARY_H

typedef enum {
    _create_udp_connection_
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

#endif