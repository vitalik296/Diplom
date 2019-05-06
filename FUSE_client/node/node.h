#ifndef DIPLOM_NODE_H
#define DIPLOM_NODE_H

#endif //DIPLOM_NODE_H

#define SOCKET unsigned int
#define HOST "0.0.0.0"
#define PORT 12001
#define CONNECTIONS_DEEP 5

typedef struct {
    storage_t* storage;
    struct sockaddr_in * node_sock;
    SOCKET node_descriptor;
} node_t;

node_t* node_init();
void node_create(node_t* node, int port);
void node_bind(node_t* node);
void node_listen(node_t* node);
int node_accept(node_t* node, struct sockaddr* client_addr, int* addr_len);
void node_free(node_t* node);

void node_write(node_t* node, void* buffer, size_t byte_count, int block_offset, int in_block_offset);