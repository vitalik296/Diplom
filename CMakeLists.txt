cmake_minimum_required(VERSION 3.13)
project(diplom C)

set(CMAKE_C_STANDARD 11)

add_library(node node.c node.h storage.c storage.h)

add_executable(diplom main.c)

target_link_libraries(diplom node)

if(WIN32 OR _WIN32)
    target_link_libraries(diplom wsock32 ws2_32)
endif()