//
// Created by Zaharchenko on 11.03.2019.
//

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "storage.h"
#include "node.h"


node_t* node_init(){
    // Node initializer
    node_t* node = malloc(sizeof(node_t));
    node->storage = storage_init();
    node->node_sock = malloc(sizeof(struct sockaddr_in));

    node->node_sock->sin_family = AF_INET;
    node->node_sock->sin_port = htons((u_short) PORT);
    node->node_sock->sin_addr.s_addr = INADDR_ANY;

    return node;
}

void node_create(node_t* node, int port){ //TODO add functionality for port argument(will used to split servers socket ans clients socket)
    // Creates node socket
    SOCKET sock_desc = (unsigned int) socket(AF_INET, SOCK_STREAM, 0);

    if(sock_desc == SOCKET_ERROR) {
        perror(strerror(errno));
        exit(1);
    }
    node->node_descriptor = sock_desc;
}

void node_bind(node_t* node){
    // Bind socket. ONLY FOR SERVER!!!
    if(bind(node->node_descriptor, (struct sockaddr *) node->node_sock, sizeof(struct sockaddr_in))) {
        perror(strerror(errno));
        exit(1);
    }
}

void node_listen(node_t* node){
    // Change socket status to "listen". ONLY FOR SERVER!!!
    if(listen(node->node_descriptor, CONNECTIONS_DEEP) == -1 ){
        perror(strerror(errno));
        exit(1);
    }
}

SOCKET node_accept(node_t* node, struct sockaddr* client_addr, int* struct_len){
    // Accepts connection to server. ONLY FOR SERVER!!!
    SOCKET client_fd;
    if ((client_fd = (unsigned int) accept(node->node_descriptor, NULL, NULL)) == -1){ //TODO second and third parameters
        perror(strerror(errno));
        exit(1);
    }
    return client_fd;
}

void node_free(node_t* node){
    // node destructor
    storage_destroy(node->storage);
    free(node->node_sock);
    close(node->node_descriptor);
    free(node);
}

void node_write(node_t* node, void * buffer, size_t byte_count, int block_offset, int in_block_offset){
    // ONLY FOR TEST!!
    storage_write(node->storage, buffer, byte_count, block_offset, in_block_offset);
}

void node_read(node_t* node, void * buffer, size_t byte_count, int block_offset, int in_block_offset){
    // ONLY FOR TEST!!
    storage_read(node->storage, buffer, byte_count, block_offset, in_block_offset);

}

