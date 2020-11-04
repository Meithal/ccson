cmake_minimum_required(VERSION 3.14)
project(json C)

set(CMAKE_C_STANDARD 11)

add_library(njson SHARED json.h njson.c)

add_executable(main main.c)

target_link_libraries(main njson)