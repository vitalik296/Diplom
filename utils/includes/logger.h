//
// Created by Zaharchenko on 17.04.2019.
//

#ifndef UTILS_LOGGER_H
#define UTILS_LOGGER_H

#define DEFAULT_LOG "diplom.log"

#include "common.h"

typedef enum {
    INFO, WARNING, ERROR
} log_type;

typedef struct {
    int log_d;
    method_t* write_log;
    method_t* destroy;
} logger_t;

logger_t* logger_init(char* path_name);

#endif //UTILS_LOGGER_H
