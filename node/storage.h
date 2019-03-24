//
// Created by Zaharchenko on 27.02.2019.
//

#ifndef DIPLOM_NODE_H
#define DIPLOM_NODE_H
#define STORAGE_SIZE 1024*1024
#define BLOCK_SIZE 512
#define STATUS short int

typedef struct __storage_s{
    long int filled_memory;
    long int size;
    long int seek;
    int block_size;
    int descriptor;
} storage_t;

typedef struct{
    size_t buffer_size;
    void* buffer;
    int block_offset;
    int in_block_offset;
} buffer_t;

storage_t* storage_init();
void storage_destroy(storage_t* storage_to_delete);
int storage_read(storage_t* stor, buffer_t * buffer);
int storage_write(storage_t* stor, buffer_t * buffer);
STATUS storage_seek(storage_t* stor,int seek);


#endif //DIPLOM_NODE_H

