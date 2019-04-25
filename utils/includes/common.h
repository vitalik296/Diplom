//
// Created by Zaharchenko on 17.04.2019.
//

#ifndef UTILS_COMMON_H
#define UTILS_COMMON_H

typedef enum { false, true } bool;
typedef void* method_t(void* this, ...);

void throw(void);

#endif //UTILS_COMMON_H
