#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <signal.h>

#include "utils.h"


//#include "storage.h"
//#include "node.h"
//
//#include "server.h"

//#include "interaction.h"

int main(void) {

    queue_t* queue = queue_init();

    char* test ="skdfsdoiufoiaufjmxc,nvbv;reoiugiofdghxcklhgjbiruheioghzxg,mznvlierhughbvkjdfshgreiugkjfhgjskdfgh,mbvn,dfnsglkureltugkjmcnvbjkdfh";

    transmit_data(test, strlen(test), 0, queue, NULL);

    queue->destroy(queue);

    logger_t* logger = logger_init(NULL);

    logger->write_log(logger, "test", INFO);
    logger->write_log(logger, "test", WARNING);
    logger->write_log(logger, "test", ERROR);

    logger->destroy(logger);

    return 0;
}
