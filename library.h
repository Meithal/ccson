#ifndef JSON_LIBRARY_H
#define JSON_LIBRARY_H
#include<stdio.h>
#include<stdlib.h>

#include<stdbool.h>

#include<string.h>


/**
 * Json structures are stored as a flat array of objects holding
 * a link to their parent whose value is their index in that array, index zero being the root
 * Json doesn't require json arrays elements to have an order so sibling data is not stored
 */


struct node {
    enum kind {
        object,
        array,
        number,
        string,
        json_true,
        json_false,
        json_null
    } kind;
};

extern enum json_errors {
    JSONERR_NO_ERRORS = 0,
    JSONERR_INVALID_CHARACTER
} json_error;

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

#define X(a, b) a,
enum structural_tokens { STRUCTURAL };
enum literal_name_tokens { LITERAL };
enum whitespace_tokens { WHITESPACE };
#undef X

#define X(a, b) [a] = b,
char structural_tokens[] = {
    STRUCTURAL
};

char * literal_name_tokens[] = {
    LITERAL
};

char whitespace_tokens[] = {
    WHITESPACE '\0'
};
#undef X

#undef STRUCTURAL
#undef LITERAL
#undef WHITESPACE

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

void hello(void);
char * encode_json(struct node node);
struct node * decode_json(const char * str);
void free_json(struct node * node);
void free_json_str(char * json_str);

#endif //JSON_LIBRARY_H
