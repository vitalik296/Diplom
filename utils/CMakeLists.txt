cmake_minimum_required(VERSION 3.13)
project(utils C)

set(CMAKE_C_STANDARD 11)

add_library(utils utils.c utils.h queue.c logger.c)

install(TARGETS utils DESTINATION lib)
install(FILES utils.h DESTINATION include)
