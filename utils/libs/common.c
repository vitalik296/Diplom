#include "common.h"

void throw() {
    perror(strerror(errno));
    exit(EXIT_FAILURE);
}

uint16_t hash_function(uint8_t* data, size_t size) {
    uint16_t crc = 0;

    while (size--) {
        crc ^= *data++;
        for (int i = 0; i < 8; i++) {
            crc = (uint16_t) (crc & 1 ? (crc >> 1) ^ POLINOM : crc >> 1);
        }
    }

    return crc;
}
