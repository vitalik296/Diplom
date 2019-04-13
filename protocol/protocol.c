#include "protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#define BUFFER_SIZE 512

#define TRANSFER_BUFFER_SIZE 4800
#define PDU_SIZE 120
#define MSS_SIZE 60
#define PDU_HEADER 40
#define L2_HEADER_SIZE 20
#define FILE_EXIST_ERROR 17
#define RESPONSE_BUFFER_SIZE 5

#define POLINOM 0x8005
#define TTR 16

#define MAX_DATA_SIZE 512

//#define HEADER_SIZE 113
#define HEADER_SIZE 15

//uint16_t crc16(uint16_t crc, uint8_t* a, size_t len) {
//    while (len--) {
//        crc ^= *a++;
//        for (int i = 0; i < 8; ++i) {
//            crc = (uint16_t) (crc & 1 ? (crc >> 1) ^ POLINOM : crc >> 1);
//        }
//    }
//    return crc;
//}

void pack(void* package, size_t package_size);

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

void protocol_transmit(void* data, size_t size) {
    void* package = data;

    do {
        size_t package_size = (size < MAX_DATA_SIZE) ? size : MAX_DATA_SIZE;
        pack(package, package_size);
        package += package_size;
        size -= package_size;
    } while (size > 0);
}

void pack(void* package, size_t package_size) {

}

void transmitter_layer2(int32_t node, void* data, size_t size, int last) {
    size_t package_size = HEADER_SIZE + size;
    void* package = malloc(package_size);
    memset(package, 0, package_size);

    memcpy(package, node, 4);
    memcpy(package + 4, package_n, 4);
    memcpy(package + 8, end, 1); // lframe
    memcpy(package + 11, size, 4);
    memcpy(package + 15, data, size);

    uint16_t* checksum = crc16(package, package_size);
    memcpy(package + 9, checksum, 2);
    free(checksum);

    transmitter_layer1();

    free(package);
}

void transmitter_layer1() {

}
