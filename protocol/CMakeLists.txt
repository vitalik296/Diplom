cmake_minimum_required(VERSION 3.13)
project(protocol C)

set(CMAKE_C_STANDARD 11)

add_library(protocol library.c library.h)

link_directories(libs)
include_directories(includes)

target_link_libraries(protocol utils)