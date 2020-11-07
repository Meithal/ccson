#! cmake -P

message(STATUS "Generating single header file")
file(READ json.h header)
file(READ njson.c body OFFSET 18)
file(WRITE single_header.h "/* Automatically generated to be used as a single header replacement
 * of the full library, do not edit. */")
file(APPEND single_header.h "
${header}")
file(APPEND single_header.h "${body}")
message(STATUS "Done. ~> single_header.h")
