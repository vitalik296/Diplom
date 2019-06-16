cmake_minimum_required(VERSION 3.13)
project(utils C)

set(CMAKE_C_STANDARD 99)

link_directories(libs)
include_directories(includes)

add_library(
        utils utils.h
        libs/common.c includes/common.h
        libs/queue.c includes/queue.h
        libs/logger.c includes/logger.h
        libs/libsocket.c includes/libsocket.h
        libs/protocol.c includes/protocol.h
        libs/common.c includes/common.h
        libs/hashtable.c includes/hashtable.h
        libs/list.c includes/list.h
        libs/transmitter.c includes/transmitter.h
)
