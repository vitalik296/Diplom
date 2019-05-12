//
// Created by Zaharchenko on 17.04.2019.
//
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "common.h"

void throw(void) {
    perror(strerror(errno));
    exit(EXIT_FAILURE);
}

uint16_t* hash_function(void* data_to_hash, size_t len) {

    void* a = malloc(len);
    memcpy(a, data_to_hash, len);

    uint16_t* crc = malloc(2);
    memset(crc, 0, 2);

    while (len--) {
        *crc ^= *(uint8_t*)a++;
        for (int i = 0; i < 8; ++i) {
            *crc = (uint16_t) (*crc & 1 ? (*crc >> 1) ^ POLINOM : *crc >> 1);
        }
    }

    return crc;
}
