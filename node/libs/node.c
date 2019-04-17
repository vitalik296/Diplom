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

//node_t* node_init() {
//    // Node initializer
//    node_t* node = malloc(sizeof(node_t));
//    node->storage = storage_init();
//    node->node_sock = socket_addr_init(AF_INET, HOST, PORT);
//    return node;
//}
//
//void node_create(node_t* node, int port) {
//    // Creates node socket
//    node->node_descriptor = (unsigned int) socket_create_tcp();
//}
//
//void node_bind(node_t* node) {
//    // Bind socket. ONLY FOR SERVER!!!
//    socket_bind(node->node_descriptor, node->node_sock);
//}
//
//void node_listen(node_t* node) {
//    // Change socket status to "listen". ONLY FOR SERVER!!!
//    socket_listen(node->node_descriptor, CONNECTIONS_DEEP);
//}
//
//int node_accept(node_t* node, struct sockaddr* client_addr, int* addr_len) {
//    // Accepts connection to server. ONLY FOR SERVER!!!
//    return socket_accept(node->node_descriptor, client_addr, addr_len);
//}
//
//void node_free(node_t* node) {
//    // node destructor
//    storage_destroy(node->storage);
//    free(node->node_sock);
//    close(node->node_descriptor);
//    free(node);
//}
//
//
//
//void node_write(node_t* node, void* buffer, size_t byte_count, int block_offset, int in_block_offset) {
//    // ONLY FOR TEST!!
//    storage_write(node->storage, buffer, byte_count, block_offset, in_block_offset);
//}
//
//void node_read(node_t* node, void* buffer, size_t byte_count, int block_offset, int in_block_offset) {
//    // ONLY FOR TEST!!
//    storage_read(node->storage, buffer, byte_count, block_offset, in_block_offset);
//
//}