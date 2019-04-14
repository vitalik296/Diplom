#ifndef PROTOCOL_LIBRARY_H
#define PROTOCOL_LIBRARY_H

#include "utils.h"

#define MAX_DATA_SIZE 50
#define HEADER_SIZE 15
#define RESPONSE_BUFFER_SIZE 5
#define TTR 16

#define POLINOM 0x8005

void transmit_data(void* data, size_t size, int fd, queue_t* queue, void* hash_table);

#endif