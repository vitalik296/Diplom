cmake_minimum_required(VERSION 3.13)
project(tcp C)

set(CMAKE_C_STANDARD 11)

add_library(tcp tcp.c tcp.h)

link_directories(libs)
include_directories(includes)

target_link_libraries(tcp libinteraction.a)
target_link_libraries(tcp liblibsocket.a)
target_link_libraries(tcp libutils.a)