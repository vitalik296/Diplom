//
// Created by Zaharchenko on 17.04.2019.
//

#ifndef UTILS_PROTOCOL_H
#define UTILS_PROTOCOL_H

#include <sys/types.h>

#include "queue.h"

#define MAX_DATA_SIZE 50
#define HEADER_SIZE 15
#define RESPONSE_BUFFER_SIZE 5
#define TTR 16
#define POLINOM 0x8005

void transmit_data(void* data, size_t size, int fd, queue_t* queue, void* hash_table);


#endif //UTILS_PROTOCOL_H
