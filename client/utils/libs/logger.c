#include "logger.h"

logger_t* logger_init(char* pathname) {
    logger_t* logger = malloc(sizeof(logger_t));

    int flags = O_CREAT | O_WRONLY | O_APPEND;

    if (pathname == NULL) {
        logger->fd = open(DEFAULT_LOG, flags, 0777);
    } else {
        logger->fd = open(pathname, flags, 0777);
    }

    return logger;
}


void logger_write(logger_t* logger, char* text, log_type type) {
    char* error = strerror(errno);

    char* log_text = NULL;
    size_t log_text_size = 0;
    char* log_type_text = NULL;
    time_t lt = time(NULL);
    char* time_text = asctime(localtime(&lt));

    switch (type) {
        case ERROR:
            log_type_text = "[ERROR] ";
            log_text_size = strlen(log_type_text) + strlen(time_text) + strlen(text) + strlen(error) + 5; //+1
            log_text = calloc(log_text_size, sizeof(char));
            sprintf(log_text, "%s%s\t%s: %s\n", log_type_text, time_text, text, error);
            break;
        case WARNING:
            log_type_text = "[WARNING] ";
            log_text_size = strlen(log_type_text) + strlen(time_text) + strlen(text) + 3; //+1
            log_text = calloc(log_text_size, sizeof(char));
            sprintf(log_text, "%s%s\t%s\n", log_type_text, time_text, text);
            break;
        case INFO:
            log_type_text = "[INFO] ";
            log_text_size = strlen(log_type_text) + strlen(time_text) + strlen(text) + 3; //+1
            log_text = calloc(log_text_size, sizeof(char));
            sprintf(log_text, "%s%s\t%s\n", log_type_text, time_text, text);
            break;
    }

    if (log_text != NULL) {
        write(logger->fd, log_text, log_text_size - 1);
    }

    free(log_text);
}

void logger_printf(logger_t * logger, size_t size, const char* format, ...) {
    char* buffer = calloc(size, sizeof(char));

    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    logger_write(logger, buffer, INFO);

    free(buffer);
}

void logger_free(logger_t* logger) {
    close(logger->fd);
    free(logger);
}
