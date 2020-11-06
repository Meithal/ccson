cmake_minimum_required(VERSION 3.14)
project(json C)

set(CMAKE_C_STANDARD 90)

add_library(njson SHARED json.h njson.c)
add_executable(main main.c)
target_link_libraries(main njson)

option(WANT_JSON1 "Add JSON 1 support." OFF)
if(WANT_JSON1)
    add_compile_definitions("WANT_JSON1")
endif()
