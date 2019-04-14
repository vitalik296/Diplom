cmake_minimum_required(VERSION 3.13)
project(protocol C)

set(CMAKE_C_STANDARD 11)

add_library(protocol protocol.c protocol.h)

link_directories(libs)
include_directories(includes)

target_link_libraries(protocol libinteraction.a)
target_link_libraries(protocol liblibsocket.a)
target_link_libraries(protocol libudp.a)
target_link_libraries(protocol libutils.a)