cmake_minimum_required(VERSION 3.13)
project(udp C)

set(CMAKE_C_STANDARD 99)

add_library(udp udp.c udp.h)

link_directories(libs)
include_directories(includes)

target_link_libraries(udp libinteraction.a)
target_link_libraries(udp liblibsocket.a)
target_link_libraries(udp libutils.a)