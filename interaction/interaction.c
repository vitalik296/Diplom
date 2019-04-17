#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "interaction.h"

#define INITIAL_SIZE 32

typedef struct buffer {
    void* data;
    int next;
    size_t size;
} buffer_t;

buffer_t* buffer_init() {
    buffer_t* buffer = malloc(sizeof(buffer_t));

    buffer->data = malloc(INITIAL_SIZE);
    buffer->size = INITIAL_SIZE;
    buffer->next = 0;

    return buffer;
}

void buffer_free(buffer_t* buffer) {
    free(buffer->data);
    free(buffer);
}

void reserve_space(buffer_t* buffer, size_t bytes) {
    if((buffer->next + bytes) > buffer->size) {
        buffer->data = realloc(buffer->data, buffer->size * 2);
        buffer->size *= 2;
    }
}

void serialize_int32(buffer_t* buffer, int32_t x) {
    size_t size = sizeof(int32_t);
    reserve_space(buffer, size);
    memcpy(buffer->data + buffer->next, &x, size);
    buffer->next += size;
}

void serialize_text(buffer_t* buffer, void* text, size_t size) {
    reserve_space(buffer, size);
    memcpy(buffer->data + buffer->next, text, size);
    buffer->next += size;
}

void* request_serialize(request_t* request) {
    buffer_t* buffer = buffer_init();

    serialize_int32(buffer, request->operation);
    serialize_int32(buffer, request->size);
    serialize_text(buffer, request->data, request->size);

    void* output = malloc(buffer->size);
    memcpy(output, buffer->data, buffer->size);

    buffer_free(buffer);

    return output;
}


//request_t* request_deserialize(void* input) {
//    request_t* request = input;
//}





request_t* request_init(operations operation, void* data, size_t size) {
    request_t* request = malloc(sizeof(request_t));

    request->operation = operation;
    request->size = size;
    request->data = malloc(size * sizeof(void*));
    memcpy(request->data, data, size);

    return request;
}

//void request_serialize(request_t* request, char* buffer);

request_t* request_deserialize(char* filepath);

void request_free(request_t* request) {
    free(request->data);
    free(request);
}

response_t* response_init(void* data, size_t size, short status) {
    response_t* response = malloc(sizeof(response_t));

    response->status = status;
    response->size = size;
    response->data = malloc(size * sizeof(void*));
    memcpy(response->data, data, size);

    return response;
}

void response_free(response_t* response) {
    free(response->data);
    free(response);
}