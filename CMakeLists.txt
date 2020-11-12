cmake_minimum_required(VERSION 3.14)

find_package(Git QUIET)

execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-list --count HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE _var_res
        OUTPUT_VARIABLE _var_LAST_COMMIT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE _var_LAST_COMMIT_HASH
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(_var_res)
    set(_var_LAST_COMMIT, "0")
else()
    # fixme: use that instead:
    # https://cmake.org/cmake/help/latest/command/add_custom_command.html#build-events
    set_property(GLOBAL APPEND
            PROPERTY CMAKE_CONFIGURE_DEPENDS
            "${CMAKE_SOURCE_DIR}/.git/index")
endif()
add_compile_definitions(LAST_COMMIT_COUNT="${_var_LAST_COMMIT}")
add_compile_definitions(LAST_COMMIT_HASH="${_var_LAST_COMMIT_HASH}")
add_compile_definitions(CMAKE_GENERATOR="$CACHE{CMAKE_C_COMPILER}")
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

project(Cisson
        VERSION "0.0.1.${_var_LAST_COMMIT}"
        DESCRIPTION "C-(J)SON - aka JSON library in C language."
        HOMEPAGE_URL "https://gitlab.com/Meithal/cisson"
        LANGUAGES C)

set(CMAKE_C_STANDARD 90)

message(STATUS "${CMAKE_PROJECT_NAME} version ${CMAKE_PROJECT_VERSION}")


option(RUN_TESTS "Build an executable that checks the library works on your machine (recommanded)" ON)
if(RUN_TESTS)
    option(PROFILE "Profile speed of tests (Windows only)" ON)
    if(PROFILE)
        set(PROFILE_FILES tests/tests_profile.c tests/tests_profile.h)
    endif()
    add_executable(tests ${PROFILE_FILES} tests/tests.c)
    target_link_libraries(tests json)
endif()

option(WANT_SINGLE_HEADER_LIB "Generate a single header file by concatenating 2 files." ON)
if(WANT_SINGLE_HEADER_LIB)
    include(concat.cmake OPTIONAL)
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

option(WANT_LIBC "Use LibC." ON)
if(WANT_LIBC)
    add_compile_definitions("WANT_LIBC")
endif()
set(CMAKE_C_STANDARD_LIBRARIES "")
# skip all the default libraries in any case

add_library(xjson SHARED json.h njson.c)
add_library(json STATIC json.h njson.c)

