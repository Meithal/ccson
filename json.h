#ifndef JSON_JSON_H
#define JSON_JSON_H
#include<stdio.h>
#include<stdlib.h>

#include<stdbool.h>

#include<string.h>


/**
 * Json structures are stored as a flat array of objects holding
 * a link to their parent whose value is their index in that array, index zero being the root
 * Json doesn't require json arrays elements to have an order so sibling data is not stored
 */

enum json_value_kind {
    object,
    array,
    number,
    string,
    json_true,
    json_false,
    json_null
};

struct json_value {
    enum json_value_kind kind;
    int parent;
};

struct json_parsed {
    int values_count;
    struct json_value values[];
};

extern enum json_errors json_error;

#define STRUCTURAL \
  X(LEFT_SQUARE_BRACKET, '[') \
  X(LEFT_CURLY_BRACKET, '{') \
  X(RIGHT_SQUARE_BRACKET, ']') \
  X(RIGHT_CURLY_BRACKET, '}') \
  X(COLON, ':') \
  X(COMMA, ',')

#define LITERAL \
  X(JSON_TRUE, "true") \
  X(JSON_FALSE, "false") \
  X(JSON_NULL, "null")

#define WHITESPACE \
  X(TABULATION, '\t') \
  X(LINE_FEED, '\n') \
  X(CARRIAGE_RETURN, '\r') \
  X(SPACE, ' ')

#define ERRORS \
  X(JSON_ERROR_NO_ERRORS, "No errors found.") \
  X(JSON_ERROR_INVALID_CHARACTER, "Found an invalid character.") \
  X(JSON_ERROR_JSON_TOO_SHORT, "End of JSON before we could parse any meaningful token.") \
  X(JSON_ERROR_TWO_OBJECTS_HAVE_SAME_PARENT, "Two values have the same parent.") \
  X(JSON_ERROR_EMPTY, "A JSON document can't be empty.")


#define X(a, b) a,
enum structural_tokens { STRUCTURAL };
enum literal_name_tokens { LITERAL };
enum whitespace_tokens { WHITESPACE };
enum json_errors{ ERRORS };
#undef X

#define X(a, b) [a] = b,
char structurals[] = {
    STRUCTURAL
};

char * litterals[] = {
    LITERAL
};

char whitespaces[] = {
    WHITESPACE '\0'
};

char * json_errors[] = {
    ERRORS
};
#undef X

#undef STRUCTURAL
#undef LITERAL
#undef WHITESPACE
#undef ERRORS

char digits[] = "0123456789";
char digits19[] = "123456789";

enum states {
    BEFORE_TOKEN,
    AFTER_TOKEN,
    BEFORE_VALUE,
    AFTER_VALUE,
    PARSING_NULL,
    PARSING_TRUE,
    PARSING_FALSE,
    OPEN_ARRAY,
    OPEN_OBJECT
};

char * encode_json(struct json_parsed *json_parsed);
struct json_parsed * decode_json(const char *str, unsigned int length);
void free_json(struct json_parsed *json_parsed);
void free_json_str(char * json_str);
struct json_parsed * push_node(enum json_value_kind kind, int parent, struct json_parsed *json_parsed);
bool json_check_validity(struct json_parsed * json_parsed);

#endif //JSON_JSON_H
