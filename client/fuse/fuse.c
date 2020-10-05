#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <zconf.h>

#include "utils.h"

#define success 1
#define failure 0

#define tcp_port "7777"

#define DATA_SIZE 512

logger_t* logger;
socket_t* client;
socket_t* server;

sender_t* sender;

socket_t* udp_server;
logger_t* udp_logger;

hashtable_t* hashtable;

address_t* middleware_address;


char* new(size_t size) {
    return calloc(size, sizeof(char));
}


void repack(void* package, uint32_t fd, uint32_t number, uint32_t size, void* data) { // similar to pack in protocol.c
    size_t offset = 0;
    memcpy(package + offset, &fd, sizeof(fd));

    offset += sizeof(fd);
    memcpy(package + offset, &number, sizeof(number));

    offset += sizeof(number);
    memcpy(package + offset, &size, sizeof(size));

    offset += sizeof(size);
    memcpy(package + offset, data, size);

    offset = DATA_SIZE + 14 - sizeof(uint16_t);
    uint16_t checksum = hash_function(package, offset);
    memcpy(package + offset, &checksum, sizeof(checksum));
}

void transmit(uint32_t fd, void* data, uint32_t size) { // similar to protocol_transmit in protocol.c
    void* package = malloc(DATA_SIZE + 14);

    hashtable_node_t* node = hashtable_get(hashtable, &fd, sizeof(fd));

    uint32_t length;
    uint32_t offset = 0;
    uint32_t number = (uint32_t) (node != NULL ? node->value : 0);

    do {
        length = size > DATA_SIZE ? DATA_SIZE : size;

        memset(package, 0, DATA_SIZE + 14);
        repack(package, fd, number, length, data + offset);

        sender_enqueue(sender, "127.0.0.1", 4235, package, DATA_SIZE + 14);

        offset += length;
        size -= length;
        number++;

    } while (size > 0);

    hashtable_set(hashtable, &fd, sizeof(fd), number);

    free(package);
}


void client_send(char* request) {
    size_t size = strlen(request) + 1 + strlen(tcp_port);
    char* buffer = new(4 + size + 1); //+1
    memcpy(buffer, &size, 4);

    sprintf(buffer + 4, "%s&%s", request, tcp_port);

    if (logger == NULL) {
        logger_printf(logger,20, "fs: logger == NULL");
    } else {
        logger_printf(logger,20, "fs: logger != NULL");
    }
    client = socket_init_tcp(logger);
    socket_connect(client, middleware_address);
    socket_write(client, buffer, 4 + size);
    socket_free(client);
    free(buffer);
}

int check_response(char* response) {
    char* status = strtok(response, "&");

    if (status != NULL) {
        if (strcmp(status, "1") == 0) {
            return success;
        }

        char* message = strtok(NULL, "&");

        while (message != NULL) {
            logger_write(logger, message, WARNING);
            message = strtok(NULL, "&");
        }
    }

    return failure;
}

int server_recv(char* response, size_t size) {

    char* buffer = new(4 + size + 2 + 1); //+1

    socket_t* conn = socket_accept(server);
    socket_read(conn, buffer, 4 + size + 2 + 1); //+1

    int status = check_response(buffer + 4);

    logger_printf(logger,100, "fs: %d - %d", strlen(response), strlen(buffer));

    strcpy(response, buffer + 6);


    logger_printf(logger,4096, "fs: res: %s; buff: %s", response, buffer);


    socket_free(conn);
    free(buffer);

    return status;
}

int ceil(float num) {
    int inum = (int)num;
    if (num == (float)inum) {
        return inum;
    }
    return inum + 1;
}


static int fs_getattr(const char* path, struct stat* stbuf) {
    logger_printf(logger, 20, "fs: 1");
    memset(stbuf, 0, sizeof(struct stat));
    logger_printf(logger, 20, "fs: 2");
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    if (strcmp(path, "/.Trash") == 0) {
        return -ENOENT;
    }

    if (strcmp(path, "/.Trash-1000") == 0) {
        return -ENOENT;
    }

    if (strcmp(path, "/.hidden") == 0) {
        return -ENOENT;
    }

    char* command = "getattr";
    size_t request_size = strlen(command) + 1 + strlen(path);
    char* request = new(request_size + 1); //+1
    sprintf(request, "%s&%s", command, path);
    client_send(request);
    logger_printf(logger, 2048, "fs: %s (request): %s", command, request);
    free(request);

    char* response = new(1024 + 1); //+1
    int status = server_recv(response, 1024); //+1

    logger_printf(logger,2048, "fs: %s (response): %s", command, response);

    if (status == failure) {
        free(response);
        return -ENOENT;
    }

    char* type = strtok(response, "&");
    uint mode = (uint) strtoull(strtok(NULL, "&"), NULL, 0);
    uint64_t size = strtoull(strtok(NULL, "&"), NULL, 0);

    if (strcmp(type, "f") == 0) {
        stbuf->st_mode = S_IFREG | mode;
        stbuf->st_nlink = 1;
        stbuf->st_size = size;

        free(response);
        return 0;
    }

    if (strcmp(type, "d") == 0) {
        stbuf->st_mode = S_IFDIR | mode;
        stbuf->st_nlink = 2;

        free(response);
        return 0;
    }

    free(response);
    return -ENOENT;
}

static int fs_readdir(const char* path, void* buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
    char* command = "readdir";

    size_t request_size = strlen(command) + 1 + strlen(path);

    char* request = new(request_size + 1); //+1
    sprintf(request, "%s&%s", command, path);
    client_send(request);
    logger_printf(logger, 2048, "fs: %s (request): %s", command, request);
    free(request);

    char* response = new(2550 + 9 + 1); //+1 // 10 files of 255 characters

    int status = server_recv(response, 2550 + 9 + 1); //+1

    logger_printf(logger, 2750, "fs: %s (response): %s", command, response);

    if (status == failure) {
        free(response);
        return -ENOTDIR;
    }

    filler(buffer, ".", NULL, 0);
    filler(buffer, "..", NULL, 0);

    char* filename = strtok(response, "&");

    while (filename != NULL) {
        filler(buffer, filename, NULL, 0);
        filename = strtok(NULL, "&");
    }

    free(response);

    return 0;
}

static int fs_open(const char* path, struct fuse_file_info* fi) {
    logger_printf(logger, 2048, "fs: ############### fs_open %s, %d", path, fi->flags);


    char* command = "open";

    size_t request_size = strlen(command) + 1 + strlen(path);

    char* request = new(request_size + 1); //+1
    sprintf(request, "%s&%s", command, path);
    client_send(request);
    logger_printf(logger, 2048, "fs: %s (request): %s", command, request);
    free(request);

    char* response = new(100 + 1); //+1

    int status = server_recv(response, 100 + 1); //+1

    logger_printf(logger, 2048, "fs: %s (response): %s", command, response);

    if (status == failure) {
        free(response);
        return -ENOENT;
    }

    char* descriptor = strtok(response, "&");
    char* number = strtok(NULL, "&");

    int32_t fd = (int32_t) atoi(descriptor);

    hashtable_set(hashtable, &fd, sizeof(fd), atoi(number));

    fi->fh = (uint64_t) fd;

    free(response);

    return 0;
}

static int fs_mkdir(const char* path, mode_t mode) {
    char* command = "mkdir";

    size_t request_size = strlen(command) + 1 + strlen(path);

    char* request = new(request_size + 1); //+1
    sprintf(request, "%s&%s", command, path);
    client_send(request);
    logger_printf(logger, 2048, "fs: %s (request): %s", command, request);
    free(request);

    char* response = new(1024 + 1); //+1

    int status = server_recv(response, 1024 + 1); //+1

    logger_printf(logger, 2048, "fs: %s (response): %s", command, response);

    if (status == failure) {
        free(response);
        return -ENOTDIR;
    }

    free(response);

    return 0;
}

static int fs_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
    char* command = "create";

    size_t request_size = strlen(command) + 1 + strlen(path);

    char* request = new(request_size + 1); //+1
    sprintf(request, "%s&%s", command, path);
    client_send(request);
    logger_printf(logger, 2048, "fs: %s (request): %s", command, request);
    free(request);

    char* response = new(100 + 1); //+1

    int status = server_recv(response, 100 + 1); //+1

    logger_printf(logger, 2048, "fs: %s (response): %s", command, response);

    if (status == failure) {
        free(response);
        return -ENOENT;
    }

    fi->fh = (uint64_t) atoi(response);

    free(response);

    return 0;
}

static int fs_release(const char* path, struct fuse_file_info* fi) {
    return 0;
}

static int fs_write(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info* fi) {

    logger_printf(logger, 2048, "fs: fs_write %s; %s; %d", path, buffer, size);

    int fd = -1;

    if (fi == NULL) {
    } else {
        fd = (int) fi->fh;
    }

    if (fd == -1)
        return -errno;




    float s = size;
    int count = ceil(s / DATA_SIZE);

    char* command = "write";

    size_t request_size = strlen(command) + 1 + 20 + 1 + 20;

    char* request = new(request_size + 1); //+1
    sprintf(request, "%s&%d&%d", command, fd, count);
    client_send(request);
    logger_printf(logger, 2048, "fs: %s (request): %s", command, request);
    free(request);



    transmit((uint32_t) fd, buffer, size);

    return (int) size;
}

static int fs_read(const char* path, char* buffer, size_t size, off_t offset, struct fuse_file_info* fi) {

    int fd = -1;

    if (fi == NULL) {
    } else {
        fd = (int) fi->fh;
    }

    if (fd == -1)
        return -errno;

    char* command = "read";

    size_t request_size = strlen(command) + 1 + 20 + 1 + sizeof(size) + 1 + sizeof(offset) + 4;

    char* request = new(request_size + 1); //+1
    sprintf(request, "%s&%d&%zu&%li&8888", command, fd, size, offset);
    client_send(request);
    logger_printf(logger, 2048, "fs: %s (request): %s", command, request);
    free(request);

    char* response = new(size + 1); //+1

    int status = server_recv(response, size + 1); //+1

    logger_printf(logger, 2048, "fs: %s (response): %s", command, response);

    if (status == failure) {
        free(response);
        return -ENOENT;
    }

    int count = atoi(response);

    free(response);

    int total = 0;


    void* array[count];
    int sizes[count];

    for (int i = 0; i < count; i++) {

        char* result = new(DATA_SIZE + 14 + 1);

        address_t addr;
        memset(&addr, 0, sizeof(addr));
        socklen_t addr_len = 0;

        socket_recvfrom(udp_server, result, DATA_SIZE + 14 + 1, &addr, &addr_len);


        int number;
        memcpy(&number, result + 4, 4);
        int _size;
        memcpy(&_size, result + 8, 4);



        total += _size;



        sizes[number] = _size;

        array[number] = malloc(_size+1);

        memcpy(array[number], result+12, _size);


        logger_write(udp_logger, result+12, INFO);

        free(result);

    }

    int offset_b = 0;

    for (int i = count - 1; i >= 0; i--) {
        memcpy(buffer + offset_b, array[i], sizes[i]);
        offset_b += sizes[i];
    }

    for (int i = 0; i < count; i++) {
        free(array[i]);
    }

    return total;
}

static int fs_truncate(const char* path, off_t size) {
    logger_printf(logger, 2048, "fs: fs_truncate %s; %d", path, size);

    return 0;
}

static int fs_getxattr(const char* path, const char* name, char* value, size_t size) {
    logger_printf(logger, 2048, "fs: fs_getxattr %s; %s; %s; %d", path, name, value, size);

    return 0;
}

static struct fuse_operations fuse_example_operations = {
        .getattr = fs_getattr,
        .open = fs_open,
        .readdir = fs_readdir,
        .release = fs_release,
        .create = fs_create,
        .mkdir = fs_mkdir,
        .write = fs_write,
        .read = fs_read,
        .getxattr = fs_getxattr,
        .truncate = fs_truncate,
};

int main(int argc, char* argv[]) {

    logger = logger_init(NULL);

    hashtable = hashtable_init(150, 2);


    middleware_address = socket_addr_init(AF_INET, "0.0.0.0", 4234);

    server = socket_init_tcp(logger);
    socket_bind(server, socket_addr_init(AF_INET, "0.0.0.0", 7777));
    socket_listen(server, 10);

    udp_logger = logger_init("diplom2.log");

    udp_server = socket_init_udp(udp_logger);
    socket_bind(udp_server, socket_addr_init(AF_INET, "0.0.0.0", 8888));

    sender = sender_init(1, udp_logger);

    sender_start(sender);

    return fuse_main(argc, argv, &fuse_example_operations, NULL);
}
