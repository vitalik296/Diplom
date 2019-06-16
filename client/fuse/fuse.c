#define FUSE_USE_VERSION 26

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

//void* new(size_t size) {
//    void* self = malloc(size);
//    memset(self, 0, size);
//    return self;
//}

char* new(size_t size) {
    return calloc(size, sizeof(char));
}

logger_t* logger_create(char* pathname) {
    logger_t* logger = malloc(sizeof(logger_t));

    if (pathname == NULL) {
        logger->fd = creat(DEFAULT_LOG, 0777);
    } else {
        logger->fd = creat(pathname, 0777);
    }

    return logger;
}

///* private */
//void _create_log_text(char* log_text, char* type, char* time, char* text) {
//    strcpy(log_text, type);
//    strcat(log_text, time);
//    strcat(log_text, "\t");
//    strcat(log_text, text);
//    strcat(log_text, "\n");
//}
//
///* private */
//void _create_log_error(char* log_text, char* type, char* time, char* text, char* error) {
//    strcpy(log_text, type);
//    strcat(log_text, time);
//    strcat(log_text, "\t");
//    strcat(log_text, text);
//    strcat(log_text, ": ");
//    strcat(log_text, error);
//    strcat(log_text, "\n");
//}
//
//void logger_print(logger_t* logger, char* text, log_type type) {
//    char* error = strerror(errno);
//
//    char* log_text = NULL;
//    size_t log_text_size = 0;
//    char* log_type_text = NULL;
//    time_t lt = time(NULL);
//    char* time_text = asctime(localtime(&lt));
//
//    switch (type) {
//        case ERROR:
//            log_type_text = "[ERROR] ";
//            log_text_size = strlen(log_type_text) + strlen(time_text) + strlen(text) + strlen(error) + 4;
//            log_text = malloc(log_text_size);
//            _create_log_error(log_text, log_type_text, time_text, text, error);
//            break;
//        case WARNING:
//            log_type_text = "[WARNING] ";
//            log_text_size = strlen(log_type_text) + strlen(time_text) + strlen(text) + 2;
//            log_text = malloc(log_text_size);
//            _create_log_text(log_text, log_type_text, time_text, text);
//            break;
//        case INFO:
//            log_type_text = "[INFO] ";
//            log_text_size = strlen(log_type_text) + strlen(time_text) + strlen(text) + 2;
//            log_text = malloc(log_text_size);
//            _create_log_text(log_text, log_type_text, time_text, text);
//            break;
//    }
//
//    if (log_text != NULL) {
//        write(logger->fd, log_text, log_text_size);
//    }
//
//    free(log_text);
//}


void logger_print(logger_t* logger, char* text, log_type type) {
    char* error = strerror(errno);

    char* log_text = NULL;
    size_t log_text_size = 0;
    char* log_type_text = NULL;
    time_t lt = time(NULL);
    char* time_text = asctime(localtime(&lt));

    switch (type) {
        case ERROR:
            log_type_text = "[ERROR] ";
            log_text_size = strlen(log_type_text) + strlen(time_text) + strlen(text) + strlen(error) + 4 + 1; //+1
            log_text = calloc(log_text_size, sizeof(char));
            sprintf(log_text, "%s%s\t%s: %s\n", log_type_text, time_text, text, error);
            break;
        case WARNING:
            log_type_text = "[WARNING] ";
            log_text_size = strlen(log_type_text) + strlen(time_text) + strlen(text) + 2 + 1; //+1
            log_text = calloc(log_text_size, sizeof(char));
            sprintf(log_text, "%s%s\t%s\n", log_type_text, time_text, text);
            break;
        case INFO:
            log_type_text = "[INFO] ";
            log_text_size = strlen(log_type_text) + strlen(time_text) + strlen(text) + 2 + 1; //+1
            log_text = calloc(log_text_size, sizeof(char));
            sprintf(log_text, "%s%s\t%s\n", log_type_text, time_text, text);
            break;
    }

    if (log_text != NULL) {
        write(logger->fd, log_text, log_text_size - 1);
    }

    free(log_text);
}

void logger_printf(size_t size, const char* format, ...) {
    char* buffer = new(size);

    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    logger_print(logger, buffer, INFO);

    free(buffer);
}








typedef struct {
    address_t* address;
    char* ip;
    int port;
    void* package;
    size_t package_size;
} _queue_item_t;

_queue_item_t* _queue_item_init(char* ip, uint16_t port, void* package, size_t package_size) {
    _queue_item_t* item = malloc(sizeof(_queue_item_t));

    item->address = socket_addr_init(AF_INET, ip, port);

//    strcpy(item->ip, ip);
    item->ip = malloc(strlen(ip) + 1);
    strcpy(item->ip, ip);
    item->port = port;

    item->package = malloc(package_size);
    memcpy(item->package, package, package_size);

    item->package_size = package_size;

    return item;
}

void _queue_item_free(_queue_item_t* item) {
    free(item->address);
    free(item->package);
    free(item->ip);
    free(item);
}

void _sender_safe_enqueue(sender_t* sender, void* data, size_t size) {
    pthread_mutex_lock(&sender->mutex);

    enqueue(sender->queue, data, size);

    pthread_cond_signal(&sender->cond);

    pthread_mutex_unlock(&sender->mutex);
}
void _sender_enqueue(sender_t* sender, char* ip, uint16_t port, void* package, size_t package_size) {
    _queue_item_t* item = _queue_item_init(ip, port, package, package_size);
    _sender_safe_enqueue(sender, item, sizeof(_queue_item_t));
//    queue_item_free(item);
}
queue_node_t* _sender_safe_dequeue(sender_t* sender) {
    pthread_mutex_lock(&sender->mutex);

    while (!sender->queue->size) {
        pthread_cond_wait(&sender->cond, &sender->mutex);
    }

    queue_node_t* node = dequeue(sender->queue);

    pthread_mutex_unlock(&sender->mutex);

    return node;
}



void repack(void* package, uint32_t fd, uint32_t number, uint32_t size, void* data) {
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

void transmit(uint32_t fd, void* data, uint32_t size) {
    void* package = malloc(DATA_SIZE + 14);

    hashtable_node_t* node = hashtable_get(hashtable, &fd, sizeof(fd));

    uint32_t length = 0;
    uint32_t offset = 0;
    uint32_t number = (uint32_t) (node != NULL ? node->value : 0);

    do {
        if (size > DATA_SIZE) {
            length = DATA_SIZE;
        } else {
            length = size;
        }

        memset(package, 0, DATA_SIZE + 14);
        repack(package, fd, number, length, data + offset);

//        sender_enqueue(sender, "0.0.0.0", 10900, package, DATA_SIZE + 14);
        sender_enqueue(sender, "127.0.0.1", 4235, package, DATA_SIZE + 14);

        offset += length;
        size -= length;
        number++;

    } while (size > 0);

    hashtable_set(hashtable, &fd, sizeof(fd), number);

    free(package);
}








void client_send(char* request) {
//    logger_printf(20, "fs: send $1");
    size_t size = strlen(request) + 1 + strlen(tcp_port);
//    logger_printf(20, "fs: send $2");
    char* buffer = new(4 + size + 1); //+1
//    logger_printf(20, "fs: send $3");
    memcpy(buffer, &size, 4);
//    logger_printf(20, "fs: send $4");

    sprintf(buffer + 4, "%s&%s", request, tcp_port);
//    logger_printf(4 + size + 20, "fs: send: %s", request);
//    logger_printf(4 + size + 20, "fs: send: %s", buffer);
//    logger_printf(20, "fs: send $5");

    if (logger == NULL) {
        logger_printf(20, "fs: logger == NULL");
    } else {
        logger_printf(20, "fs: logger != NULL");
    }
    client = socket_init_tcp(logger);
//    logger_printf(20, "fs: send $6");
    socket_connect(client, middleware_address);
//    logger_printf(20, "fs: send $7");
    socket_write(client, buffer, 4 + size);
//    logger_printf(20, "fs: send $8");
    socket_free(client);
//    logger_printf(20, "fs: send $9");
    free(buffer);
//    logger_printf(20, "fs: send $10");
}

int check_response(char* response) {
    char* status = strtok(response, "&");

    if (status != NULL) {
        if (strcmp(status, "1") == 0) {
            return success;
        }

        char* message = strtok(NULL, "&");

        while (message != NULL) {
            logger_print(logger, message, WARNING);
            message = strtok(NULL, "&");
        }
    }

    return failure;
}

int server_recv(char* response, size_t size) {
//    char* buffer = new(size + 2);
//
//    socket_t* conn = socket_accept(server);
//    socket_read(conn, buffer, size + 2);
//
//    int status = check_response(buffer);
//    memcpy(response, buffer + 2, size);
//
//    socket_free(conn);

//    logger_printf(20, "fs: recv #1");
    char* buffer = new(4 + size + 2 + 1); //+1

//    logger_printf(20, "fs: recv #2");
    socket_t* conn = socket_accept(server);
//    logger_printf(20, "fs: recv #3");
    socket_read(conn, buffer, 4 + size + 2 + 1); //+1
//    logger_printf(20, "fs: recv #4");

    int status = check_response(buffer + 4);
//    logger_printf(20, "fs: recv #5");

    logger_printf(100, "fs: %d - %d", strlen(response), strlen(buffer));

//    memcpy(response, buffer + 6, size);
    strcpy(response, buffer + 6);

//    logger_printf(20, "fs: recv #6");

    logger_printf(4096, "fs: res: %s; buff: %s", response, buffer);

//    logger_printf(20, "fs: recv #6.1");

    socket_free(conn);
//    logger_printf(20, "fs: recv #7");
    free(buffer);
//    logger_printf(20, "fs: recv #8");

    return status;
}

// operations //

static int fs_getattr(const char* path, struct stat* stbuf) {
    logger_printf(20, "fs: 1");
    memset(stbuf, 0, sizeof(struct stat));
    logger_printf(20, "fs: 2");
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

//    logger_printf(20, "fs: 3");
    char* command = "getattr";
//    logger_printf(20, "fs: 4");
    size_t request_size = strlen(command) + 1 + strlen(path);
//    logger_printf(20, "fs: 5");
    char* request = new(request_size + 1); //+1
//    logger_printf(20, "fs: 6");
    sprintf(request, "%s&%s", command, path);
//    logger_printf(20, "fs: 7");
    client_send(request);
//    logger_printf(20, "fs: 7.1");
    logger_printf(2048, "fs: %s (request): %s", command, request);
//    logger_printf(20, "fs: 8");
    free(request);
//    logger_printf(20, "fs: 9");

    char* response = new(1024 + 1); //+1
//    logger_printf(20, "fs: 10");
    int status = server_recv(response, 1024); //+1
//    logger_printf(20, "fs: 11");

    logger_printf(2048, "fs: %s (response): %s", command, response);

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
    logger_printf(2048, "fs: %s (request): %s", command, request);
    free(request);

    char* response = new(2550 + 9 + 1); //+1 // 10 files of 255 characters

    int status = server_recv(response, 2550 + 9 + 1); //+1

    logger_printf(2750, "fs: %s (response): %s", command, response);

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
    logger_printf(2048, "fs: ############### fs_open %s, %d", path, fi->flags);


    char* command = "open";

    size_t request_size = strlen(command) + 1 + strlen(path);

    char* request = new(request_size + 1); //+1
    sprintf(request, "%s&%s", command, path);
    client_send(request);
    logger_printf(2048, "fs: %s (request): %s", command, request);
    free(request);

    char* response = new(100 + 1); //+1

    int status = server_recv(response, 100 + 1); //+1

    logger_printf(2048, "fs: %s (response): %s", command, response);

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
    logger_printf(2048, "fs: %s (request): %s", command, request);
    free(request);

    char* response = new(1024 + 1); //+1

    int status = server_recv(response, 1024 + 1); //+1

    logger_printf(2048, "fs: %s (response): %s", command, response);

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
    logger_printf(2048, "fs: %s (request): %s", command, request);
    free(request);

    char* response = new(100 + 1); //+1

    int status = server_recv(response, 100 + 1); //+1

    logger_printf(2048, "fs: %s (response): %s", command, response);

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
int ceil(float num) {
    int inum = (int)num;
    if (num == (float)inum) {
        return inum;
    }
    return inum + 1;
}
static int fs_write(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info* fi) {

    logger_printf(2048, "fs: fs_write %s; %s; %d", path, buffer, size);

    int fd = -1;
//    int res;

    if (fi == NULL) {
//        fd = open(path, O_WRONLY);
    } else {
        fd = (int) fi->fh;
    }

    if (fd == -1)
        return -errno;


//    int count = (int) ceil(size / DATA_SIZE); // ПРОВЕРИТЬ НА НОЛЬ
//
//    char* command = "write";
//
//    size_t request_size = strlen(command) + 1 + strlen(path) + 20 + strlen(buffer) + 20;
//
//    char* request = new(request_size + 1); //+1
//    sprintf(request, "%s&%s&%s&%d&%d", command, path, buffer, count, fi->flags);
//    client_send(request);
//    logger_printf(2048, "fs: %s (request): %s", command, request);
//    free(request);
//
//    char* response = new(100 + 1); //+1
//
//    int status = server_recv(response, 100 + 1); //+1
//
//    logger_printf(2048, "fs: %s (response): %s", command, response);

//    if (status == failure) {
//        free(response);
//        return -ENOENT;
//    }

    float s = size;
    int count = ceil(s / DATA_SIZE);

    char* command = "write";

    size_t request_size = strlen(command) + 1 + 20 + 1 + 20;

    char* request = new(request_size + 1); //+1
    sprintf(request, "%s&%d&%d", command, fd, count);
    client_send(request);
    logger_printf(2048, "fs: %s (request): %s", command, request);
    free(request);

//    _sender_enqueue(sender, "0.0.0.0", 10900, buffer, strlen(buffer));
//    sender_enqueue(sender, "0.0.0.0", 10900, buffer, strlen(buffer));

//    buffer

    transmit((uint32_t) fd, buffer, size);

    return (int) size;
}

static int fs_read(const char* path, char* buffer, size_t size, off_t offset, struct fuse_file_info* fi) {

    int fd = -1;
//    int res;

    if (fi == NULL) {
//        fd = open(path, O_WRONLY);
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
    logger_printf(2048, "fs: %s (request): %s", command, request);
    free(request);

    char* response = new(size + 1); //+1

    int status = server_recv(response, size + 1); //+1

    logger_printf(2048, "fs: %s (response): %s", command, response);

    if (status == failure) {
        free(response);
        return -ENOENT;
    }

//    memcpy(buffer, response, size);
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

//        int number = atoi(result + 4);

//        int _size = atoi(result + 8);

        total += _size;



        sizes[number] = _size;

        array[number] = malloc(_size+1);

        memcpy(array[number], result+12, _size);


        logger_print(udp_logger, result+12, INFO);

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
    logger_printf(2048, "fs: fs_truncate %s; %d", path, size);

    return 0;
}

static int fs_getxattr(const char* path, const char* name, char* value, size_t size) {
    logger_printf(2048, "fs: fs_getxattr %s; %s; %s; %d", path, name, value, size);

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


int _safe_sendto(socket_t* server, _queue_item_t* item) {
    for (int ttr = 0; ttr < TTR; ttr++) {
        address_t* address = socket_addr_init(AF_INET, item->ip, (uint16_t) item->port);
        if (socket_sendto(server, item->package, item->package_size, address, sizeof(address_t)) > 0) {
            return 1;
        }
    }

    return 0;
}
void udp_h(sender_t* sender) {
    int status, answer;
    queue_node_t* node;
    _queue_item_t* item;

    socket_t* server = socket_init_udp(sender->logger);
    socket_timeout(server, 10, 0);

    while (1) {
        status = 0;

        node = _sender_safe_dequeue(sender);
        item = node->data;

        for (int ttr = 0; ttr < TTR; ttr++) {
            if (_safe_sendto(server, item) == 0) {
                socket_log(server, "Can not sendto client", WARNING);
                continue;
            }

            if (socket_recvfrom(server, &answer, sizeof(answer), NULL, NULL) < 0) {
                continue;
            }

            if (answer == 0) {
                socket_log(server, "Package status is incorrect", WARNING);
                continue;
            }

            status = 1;

            break;
        }

        if (status == 0) {
            _sender_safe_enqueue(sender, item, sizeof(_queue_item_t));
            socket_log(server, "Can not sendto or recvfrom package", WARNING);
        } else {
//            _queue_item_free(item);
        }

        queue_node_free(node);
    }
}

void sender_s(sender_t* sender) {
    for (int i = 0; i < sender->number; i++) {
        if (pthread_create(&sender->threads[i], NULL, (void*) udp_h, sender) != 0) {
            logger_write(sender->logger, "sender_start", ERROR);
        }
    }
}

int main(int argc, char* argv[]) {

    logger = logger_create(NULL);

    hashtable = hashtable_init(150, 2);


//    middleware_address = socket_addr_init(AF_INET, "0.0.0.0", 14900);
    middleware_address = socket_addr_init(AF_INET, "0.0.0.0", 4234);

    server = socket_init_tcp(logger);
    socket_bind(server, socket_addr_init(AF_INET, "0.0.0.0", 7777));
    socket_listen(server, 10);

    udp_logger = logger_create("diplom2.log");

    udp_server = socket_init_udp(udp_logger);
    socket_bind(udp_server, socket_addr_init(AF_INET, "0.0.0.0", 8888));

    sender = sender_init(1, udp_logger);

    sender_start(sender);

//    sender_s(sender);

    return fuse_main(argc, argv, &fuse_example_operations, NULL);
}
