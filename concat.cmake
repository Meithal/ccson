#!cmake -P

message(STATUS "Generating single header file")

file(READ json.h _var_header)
string(REPLACE "#endif //JSON_JSON_H" "/* Stripped header guard. */" _var_header "${_var_header}")
file(READ njson.c _var_body OFFSET 18)
file(WRITE cisson.h "/* Automatically generated to be used as a single header replacement
 * of the full library, do not edit. */")
file(APPEND cisson.h "\n${_var_header}")
file(APPEND cisson.h "\n#endif // JSON_JSON_H /* Automatically added header guard. */
")
file(APPEND cisson.h "#ifdef CISSON_IMPLEMENTATION")
file(APPEND cisson.h "${_var_body}")
file(APPEND cisson.h "#endif  // CISSON_IMPLEMENTATION")

message(STATUS "Done. ~> cisson.h")
