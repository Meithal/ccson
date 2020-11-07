#! cmake -P

message(STATUS "Generating single header file")
file(READ json.h header)
file(READ njson.c body OFFSET 18)
file(WRITE single_header.h "/* Automatically generated, do not edit. */")
file(APPEND single_header.h "
${header}")
file(APPEND single_header.h "${body}")
message(STATUS "Done. ~> single_header.h")
