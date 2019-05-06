cmake_minimum_required(VERSION 3.13)
project(diplom C)

set(CMAKE_C_STANDARD 11)

add_library(node libs/node.c libs/storage.c server.c server.h)

link_directories(libs)
include_directories(includes)

add_executable(diplom main.c)


target_link_libraries(diplom libutils.a)

#target_link_libraries(diplom node)
##target_link_libraries(diplom libinteraction.a)
#target_link_libraries(diplom libudp.a)
##target_link_libraries(diplom libtcp.a)
##target_link_libraries(diplom liblibsocket.a)

#if(WIN32 OR _WIN32)
#    target_link_libraries(diplom wsock32 ws2_32)
#endif()