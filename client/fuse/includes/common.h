#ifndef UTILS_COMMON_H
#define UTILS_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#define POLINOM 0x8005

typedef void* method_t(void* this, ...);

void throw();
uint16_t hash_function(uint8_t* data, size_t len);

#endif //UTILS_COMMON_H