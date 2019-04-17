#include <string.h>
#include <sys/fcntl.h>
#include <error.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

#include "logger.h"

void create_log_text(char* log_text, char* user_text, char* time, char* type) {
    strcpy(log_text, type);
    strcat(log_text, time);
    strcat(log_text, "\t");
    strcat(log_text, user_text);
    strcat(log_text, "\n");
}


void write_log(logger_t* this, char* text, log_type type) {

    struct tm* ptr;
    time_t lt;
    lt = time(NULL);
    ptr = localtime(&lt);
    char* log_text = NULL;
    size_t log_text_len = 0;

    switch (type) {
        case ERROR:
            log_text_len = strlen("[ERROR] ") + strlen(asctime(ptr)) + 2 + strlen(strerror(errno));
            log_text = malloc(log_text_len);
            create_log_text(log_text, strerror(errno), asctime(ptr), "[ERROR] ");
            break;
        case WARNING:
            log_text_len = strlen("[WARNING] ") + strlen(asctime(ptr)) + 2 + strlen(text);
            log_text = malloc(log_text_len);
            create_log_text(log_text, text, asctime(ptr), "[WARNING] ");
            break;
        case INFO:
            log_text_len = strlen("[INFO] ") + strlen(asctime(ptr)) + 2 + strlen(text);
            log_text = malloc(log_text_len);
            create_log_text(log_text, text, asctime(ptr), "[INFO] ");
            break;

    }

    write(this->log_d, log_text, log_text_len);
    free(log_text);

}

void logger_destroy(logger_t* this) {
    close(this->log_d);
    free(this);
}


logger_t* logger_init(char* path_name) {
    logger_t* logger = malloc(sizeof(logger_t));


    if (path_name == NULL)
        logger->log_d = open(DEFAULT_LOG, O_CREAT | O_RDWR | O_APPEND, 0777);
    else
        logger->log_d = open(path_name, O_CREAT | O_RDWR | O_APPEND, 0777);

    logger->write_log = (method_t*) write_log;
    logger->destroy = (method_t*) logger_destroy;

    return logger;
}

