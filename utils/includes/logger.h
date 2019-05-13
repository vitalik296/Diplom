#ifndef UTILS_LOGGER_H
#define UTILS_LOGGER_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define DEFAULT_LOG "diplom.log"

typedef enum { INFO, WARNING, ERROR } log_type;

typedef struct {
    int fd;
} logger_t;

logger_t* logger_init(char* pathname);
void logger_write(logger_t* logger, char* text, log_type type);
void logger_free(logger_t* logger);

#endif //UTILS_LOGGER_H