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

/* private */
void create_log_text(char* log_text, char* type, char* time, char* text) {
    strcpy(log_text, type);
    strcat(log_text, time);
    strcat(log_text, "\t");
    strcat(log_text, text);
    strcat(log_text, "\n");
}

/* private */
void create_log_error(char* log_text, char* type, char* time, char* text, char* error) {
    strcpy(log_text, type);
    strcat(log_text, time);
    strcat(log_text, "\t");
    strcat(log_text, text);
    strcat(log_text, ": ");
    strcat(log_text, error);
    strcat(log_text, "\n");
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
            log_text_size = strlen(log_type_text) + strlen(time_text) + strlen(text) + strlen(error) + 4;
            log_text = malloc(log_text_size);
            create_log_error(log_text, log_type_text, time_text, text, error);
            break;
        case WARNING:
            log_type_text = "[WARNING] ";
            log_text_size = strlen(log_type_text) + strlen(time_text) + strlen(text) + 2;
            log_text = malloc(log_text_size);
            create_log_text(log_text, log_type_text, time_text, text);
            break;
        case INFO:
            log_type_text = "[INFO] ";
            log_text_size = strlen(log_type_text) + strlen(time_text) + strlen(text) + 2;
            log_text = malloc(log_text_size);
            create_log_text(log_text, log_type_text, time_text, text);
            break;
    }

    if (log_text != NULL) {
        write(logger->fd, log_text, log_text_size);
    }

    free(log_text);
}

void logger_free(logger_t* logger) {
    close(logger->fd);
    free(logger);
}
