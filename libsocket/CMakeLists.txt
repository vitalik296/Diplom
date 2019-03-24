cmake_minimum_required(VERSION 3.13)
project(libsocket C)

set(CMAKE_C_STANDARD 11)

add_library(libsocket libsocket.c libsocket.h)

link_directories(libs)
include_directories(includes)

target_link_libraries(libsocket utils)
