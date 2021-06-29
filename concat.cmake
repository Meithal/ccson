#!cmake -P

message(STATUS "Generating single header file")

file(READ ../json.h _var_header)
string(REPLACE "#endif //JSON_JSON_H" "/* Stripped header guard. */" _var_header "${_var_header}")
file(READ ../njson.c _var_body OFFSET 18)
file(WRITE ../cson.h "/* Automatically generated to be used as a single header replacement
 * of the full library, do not edit. */")
file(APPEND ../cson.h "\n${_var_header}")
file(APPEND ../cson.h "\n#endif // JSON_JSON_H /* Automatically added header guard. */
")
file(APPEND ../cson.h "#ifdef CSON_IMPLEMENTATION")
file(APPEND ../cson.h "${_var_body}")
file(APPEND ../cson.h "\n#endif  // CSON_IMPLEMENTATION")

message(STATUS "Done. ~> cson.h")
