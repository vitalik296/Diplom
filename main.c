#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "storage.h"
#include "node.h"

int main() {
    void * test_buff = malloc(1024*sizeof(char));
    node_t* node = node_init();
    node_create(node, 0);
    node_bind(node);
    node_listen(node);
    puts("Waiting....");
    int client = node_accept(node, NULL, NULL);
    printf("%d\n", client);
    size_t actual_size = (size_t) recv(client, test_buff, 1024, 0);

    node_write(node, test_buff, actual_size, 0, 0);

    free(test_buff);
    node_free(node);
    close(client);

    return 0;
}