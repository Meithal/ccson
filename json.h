#ifndef JSON_JSON_H
#define JSON_JSON_H
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>


char string_pool[0x8000];
size_t string_cursor = 1;
struct token {
    enum {
        TRUE,
        FALSE,
        JNULL,
        STRING,
        NUMBER,
        ARRAY,
        OBJECT,
        OBJECT_KEY,
        OBJECT_VALUE
    } kind;
    void * root;
    void * address;
};
struct token tokens[0x200];
size_t token_cursor;

static int root_singleton;
void * roots[0x200] = {&root_singleton};
size_t root_cursor;

char arrays_pool[0x8000];
size_t array_cursor;
char assoc_pool[0x8000];
size_t assoc_cursor;
char assoc_keys_pool[0x8000];
size_t assoc_keys_cursor;

/**
 * Json structures are stored as a flat array of objects holding
 * a link to their parent whose value is their index in that array, index zero being the root
 * Json doesn't require json arrays elements to have an order so sibling data is not stored
 */

enum json_value_kind {
    json_object,
    json_array,
    json_number,
    json_string,
    json_true,
    json_false,
    json_null
};

static int JSON_TRUE_SINGLETON;
static int JSON_FALSE_SINGLETON;
static int JSON_NULL_SINGLETON;


struct json_value {
    enum json_value_kind kind;
    int parent;
    size_t * address;
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
  X(JSON_ERROR_EMPTY, "A JSON document can't be empty.") \
  X(JSON_ERROR_LEADING_PLUS, "A JSON number can't have a leading + sign.") \
  X(JSON_ERROR_WRONG_MINUS, "Found a minus sign in a JSON number in the wrong place.") \
  X(JSON_ERROR_TOO_MANY_EXPONENTS, "Found an exponent marker in a number already having one.") \
  X(JSON_ERROR_WRONG_POST_EXP, "After an exponent symbol, we expect either a +, -, or 0-9.") \
  X(JSON_ERROR_EXP_EMPTY, "The exponent part of a number can't be empty.") \
  X(JSON_ERROR_EXP_NO_DIGIT, "The exponent part of a number must have at least one digit.") \
  X(JSON_ERROR_NO_LEADING_DIGIT, "The leading part of a JSON number must have at least one digit.") \
  X(JSON_ERROR_WRONG_POST_MINUS, "After a leading minus, we expect a digit.") \
  X(JSON_ERROR_MUST_FIND_DOT_OR_EXP, "After a leading zero, we can only find a dot or an exponent.") \
  X(JSON_ERROR_TOO_MANY_DOTS, "Found a dot too many.") \
  X(JSON_ERROR_MUST_HAVE_DIGIT_AFTER_DOT, "After a decimal dot, we must have at least one digit.") \
  X(JSON_ERROR_NUMBER_WRONG_CHAR, "Rare case of an uncaught character in the exponent part of the number. Fixme.") \
  X(JSON_ERROR_INVALID_ESCAPE_SEQUENCE, "Invalid escape sequence.")                       \
  X(JSON_ERROR_INVALID_NUMBER, "Malformed number.")              \
  X(JSON_ERROR_INVALID_CHARACTER_IN_ARRAY, "Invalid character in array.")

#define X(a, b) a,
enum structural_tokens { STRUCTURAL };
enum literal_name_tokens { LITERAL };
enum whitespace_tokens { WHITESPACE };
enum json_errors{ ERRORS };
#undef X

#define X(a, b) [a] = b,
char structs[] = {
    STRUCTURAL
    '\0',
};

char * lits[] = {
    LITERAL
    NULL,
};

char whitespaces[] = {
    WHITESPACE
    '\0',
};

char * json_errors[] = {
    ERRORS
};
#undef X

#undef STRUCTURAL
#undef LITERAL
#undef WHITESPACE
#undef ERRORS

char digit_starters[] = "-0123456789";
char digits[] = "0123456789";
char digits19[] = "123456789";
char digit_glyps[] = "-+0123456789.eE";

enum states {
#ifdef OLD_FORMAT
    BEFORE_TOKEN,
    AFTER_TOKEN,
    PARSING_NULL,
    PARSING_TRUE,
    PARSING_FALSE,
#endif
    WHITESPACE_BEFORE_VALUE,
    WHITESPACE_AFTER_VALUE,
    EXPECT_VALUE,
    AFTER_VALUE,
    OPEN_ARRAY,
    OPEN_ASSOC,
    FOUND_OPEN_QUOTE,
    LITERAL_ESCAPE,
    IN_STRING,
    START_NUMBER,
    NUMBER_AFTER_MINUS,
    IN_NUMBER,
    EXPECT_FRACTION,
    EXPECT_EXPONENT,
    IN_FRACTION,
    IN_FRACTION_DIGIT,
    EXPONENT_EXPECT_PLUS_MINUS,
    EXPECT_EXPONENT_DIGIT,
    IN_EXPONENT_DIGIT,
    ARRAY_AFTER_VALUE,
    ASSOC_AFTER_VALUE,
    ASSOC_WHITESPACE_BEFORE_KEY,
};

struct state {
    enum states kind;
    int ordinal;
    enum json_errors error;
};

int rjson(char*, struct state*);

char *
 encode_json(struct json_parsed *json_parsed);
struct json_parsed *
 decode_json(const char *str, size_t length);
void
 free_json(struct json_parsed *json_parsed);
void
 free_json_str(char * json_str);
struct json_parsed *
 push_node(enum json_value_kind kind, int parent, size_t *address, struct json_parsed *json_parsed);
bool
 json_semcheck(struct json_parsed *json_parsed);
int
 json_parse_number(const char *str, size_t len);

struct json_parsed *
 ndecode_json(const char *str, unsigned int length);

#endif //JSON_JSON_H
