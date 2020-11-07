cmake_minimum_required(VERSION 3.14)
project(json C)

set(CMAKE_C_STANDARD 90)

add_library(njson SHARED json.h njson.c)


option(RUN_TESTS "Build an executable that checks the library works on your machine (recommanded)" ON)
if(RUN_TESTS)
    add_executable(main tests.c)
    target_link_libraries(main njson)
endif()

option(WANT_SINGLE_HEADER_LIB "Generate a single header file by concatenating 2 files." ON)
if(WANT_SINGLE_HEADER_LIB)
    include(concat.cmake OPTIONAL)  # todo add it as pre-commit hook
endif()

option(WANT_JSON1 "Add JSON 1 support." OFF)
if(WANT_JSON1)
    add_compile_definitions("WANT_JSON1")
endif()

option(SELF_MANAGE_MEMORY "Manage memory yourself." OFF)
if(SELF_MANAGE_MEMORY)
    add_compile_definitions("SELF_MANAGE_MEMORY")
else()
    set(STRING_POOL_SIZE  0x2000 CACHE STRING "Size of the string pool.")
    set(MAX_TOKENS        0x200  CACHE STRING "Max number of tokens the parser can store.")
    add_compile_definitions("STRING_POOL_SIZE=$CACHE{STRING_POOL_SIZE}")
    add_compile_definitions("MAX_TOKENS=$CACHE{MAX_TOKENS}")
endif()
