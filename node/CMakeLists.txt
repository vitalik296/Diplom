cmake_minimum_required(VERSION 3.13)
project(diplom C)

set(CMAKE_C_STANDARD 11)

add_library(node node.c node.h storage.c storage.h)

link_directories(libs)
include_directories(includes)

add_executable(diplom main.c)

target_link_libraries(diplom node)
target_link_libraries(diplom liblibsocket.a)
target_link_libraries(diplom libutils.a)