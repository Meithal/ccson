cmake_minimum_required(VERSION 3.14)

message(STATUS "${CMAKE_PROJECT_NAME} version ${CMAKE_PROJECT_VERSION}")

find_package(Git QUIET)

execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-list --count HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE _var_res
        OUTPUT_VARIABLE _var_LAST_COMMIT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(_var_res)
    set(_var_LAST_COMMIT, "0")
endif()

add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

project(Cisson
        VERSION "0.0.1.${_var_LAST_COMMIT}"
        DESCRIPTION "C-(J)SON - aka JSON library in C language."
        HOMEPAGE_URL "https://gitlab.com/Meithal/cisson"
        LANGUAGES C)

set(CMAKE_C_STANDARD 90 CACHE STRING "ANSI C by default")

set(JSON_FILES json.h njson.c)

option(CISSON_BUILD_SHARED ON)
option(CISSON_BUILD_STATIC ON)

if(CISSON_BUILD_SHARED)
    add_library(xjson SHARED ${JSON_FILES})
endif()
if(CISSON_BUILD_STATIC)
    add_library(sjson STATIC ${JSON_FILES})
endif()

option(CISSON_RUN_TESTS "Build an executable that checks the library works on your machine (recommended)" ON)
if(CISSON_RUN_TESTS)
    add_executable(tests tests/tests.c)
    add_executable(tests_single tests/tests.c)
    target_compile_definitions(tests_single PUBLIC CISSON_SINGLE_HEADER)
    if(CISSON_BUILD_STATIC)
        target_link_libraries(tests sjson)
    elseif(CISSON_BUILD_SHARED)
        target_link_libraries(tests xjson)
    else()
        target_sources(tests PRIVATE ${JSON_FILES})
    endif()

    if(CMAKE_PROJECT_NAME STREQUAL Cisson)
        include(CTest)
        add_test(NAME main_test COMMAND tests)
    endif()
endif()

option(CISSON_WANT_SINGLE_HEADER_LIB "Generate a single header file by concatenating 2 files." OFF)
if(CISSON_WANT_SINGLE_HEADER_LIB)
    add_custom_command(TARGET tests PRE_BUILD COMMAND cmake -P ${CMAKE_CURRENT_SOURCE_DIR}/concat.cmake VERBATIM)
endif()

option(CISSON_WANT_JSON1 "Add JSON 1 support." OFF)
if(CISSON_WANT_JSON1)
    add_compile_definitions("WANT_JSON1")
endif()

option(CISSON_SELF_MANAGE_MEMORY "Manage memory yourself." OFF)
if(CISSON_SELF_MANAGE_MEMORY)
    add_compile_definitions("SELF_MANAGE_MEMORY")
else()
    set(STRING_POOL_SIZE  0x2000 CACHE STRING "Size of the string pool.")
    set(MAX_TOKENS        0x200  CACHE STRING "Max number of tokens the parser can store.")
    add_compile_definitions("STRING_POOL_SIZE=$CACHE{STRING_POOL_SIZE}")
    add_compile_definitions("MAX_TOKENS=$CACHE{MAX_TOKENS}")
endif()

option(CISSON_WANT_LIBC "Use LibC." ON)
if(CISSON_WANT_LIBC)
    add_compile_definitions("WANT_LIBC")
endif()

set(CMAKE_C_STANDARD_LIBRARIES "")