//
// Created by Zaharchenko on 17.04.2019.
//

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "protocol.h"

uint16_t* crc16(uint8_t* a, size_t len) {
    uint16_t* crc = malloc(2);
    memset(crc, 0, 2);

    while (len--) {
        *crc ^= *a++;
        for (int i = 0; i < 8; ++i) {
            *crc = (uint16_t) (*crc & 1 ? (*crc >> 1) ^ POLINOM : *crc >> 1);
        }
    }

    return crc;
}

void data_decorator(void* data, uint32_t size, queue_t* queue, uint8_t is_last, uint32_t fd, uint32_t pack_num) {
    void* package = malloc(MAX_DATA_SIZE + HEADER_SIZE);

    memcpy(package, &fd, 4);
    memcpy(package + 4, &pack_num, 4);
    memcpy(package + 8, &is_last, 1); // lframe


    memcpy(package + 11, &size, 4);
    memcpy(package + 15, data, size);

    uint16_t* checksum = crc16(package, size);
    memcpy(package + 9, checksum, 2);
    free(checksum);


    queue->enqueue(queue, package, MAX_DATA_SIZE + HEADER_SIZE);



    free(package);

}

void transmit_data(void* data, size_t size, int fd, queue_t* queue, void* hash_table) { // TODO add hash_table
    void* package_data = malloc(MAX_DATA_SIZE);
    size_t package_size;
    size_t offset = 0;
    uint32_t pack_num = 0;
    uint8_t is_last = 0;

    do {
        if (size > MAX_DATA_SIZE) {
            package_size = MAX_DATA_SIZE;
            is_last = 0;
        } else {
            package_size = size;
            is_last = 1;
        }
        memcpy(package_data, data + offset, package_size);
        data_decorator(package_data, (uint32_t) package_size, queue, is_last, (uint32_t) fd, pack_num);
        size -= package_size;
        offset += package_size;
    } while (size > 0);
    free(package_data);
}