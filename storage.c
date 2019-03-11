//
// Created by Zaharchenko on 27.02.2019.
//

#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "storage.h"

storage_t* storage_init(){
    // Initializes storage and create(if necessary storage file)
    storage_t* new_storage = malloc(sizeof(struct __storage_s));
    new_storage->filled_memory = 0;
    new_storage->size = STORAGE_SIZE;
    new_storage->seek = 0;
    new_storage->block_size = BLOCK_SIZE;
    new_storage->descriptor = open("storage.file", O_RDWR, 0755);
    if(new_storage->descriptor == -1){
        new_storage->descriptor = open("storage.file", O_RDWR|O_CREAT, 0755);
        if (lseek(new_storage->descriptor, STORAGE_SIZE-1, SEEK_SET) == -1) {
            close(new_storage->descriptor);
            perror("Error calling lseek() to 'stretch' the file");
            return NULL;
        }

        if (write(new_storage->descriptor, "", 1) < 0) {
            close(new_storage->descriptor);
            perror("Error writing a byte at the end of the file");
            return NULL;
        }
    }

    return new_storage;
}

void storage_destroy(storage_t* storage_to_delete){
    // Storage destructor
    fsync(storage_to_delete->descriptor);
    close(storage_to_delete->descriptor);
    free(storage_to_delete);
}

int storage_read(storage_t* stor, void * buffer, size_t byte_count, int block_offset, int in_block_offset){
    // Reads a given bytes num with a given offset
    lseek(stor->descriptor, block_offset*stor->block_size+in_block_offset, SEEK_SET);
    return (int) read(stor->descriptor, buffer, byte_count);
}

int storage_write(storage_t* stor, void * buffer, size_t byte_count, int block_offset, int in_block_offset){
    // Writes a given data with a given offset
    lseek(stor->descriptor, block_offset*stor->block_size+in_block_offset, SEEK_SET);
    return (int) write(stor->descriptor, buffer, byte_count);
}

STATUS storage_seek(storage_t* stor,int seek){
    // at the moment i don't now how this function have to work
    stor->seek = seek;
    return (short) lseek(stor->descriptor, stor->seek, SEEK_SET);
}