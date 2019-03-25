#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <utils.h>

#include "storage.h"
#include "node.h"
#include "utils.h"


int main(void) {

    logger_t* logger = logger_init(NULL);

    logger->write_log(logger, "test", INFO);
    logger->write_log(logger, "test", WARNING);
    logger->write_log(logger, "test", ERROR);

    logger->destroy(logger);

//    void * test_buff;
//    node_t* node;
//
//    test_buff = malloc(1024*sizeof(char));
//    node = node_init();
//
//    node_create(node, 0);
//    node_bind(node);
//    node_listen(node);
//    puts("Waiting....");
//    int client = node_accept(node, NULL, NULL);
//    printf("%d\n", client);
//    size_t actual_size = (size_t) recv(client, test_buff, 1024, 0);
//
//    node_write(node, test_buff, actual_size, 0, 0);
//
//    free(test_buff);
//    node_free(node);
//    close(client);
    return 0;
}