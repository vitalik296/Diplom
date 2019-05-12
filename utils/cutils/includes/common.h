//
// Created by Zaharchenko on 17.04.2019.
//

#ifndef UTILS_COMMON_H
#define UTILS_COMMON_H

#define POLINOM 0x8005

typedef enum { false, true } bool;
typedef void* method_t(void* this, ...);

void throw(void);
uint16_t* hash_function(void* data_to_hash, size_t len);

#endif //UTILS_COMMON_H
