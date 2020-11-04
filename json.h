#ifndef JSON_JSON_H
#define JSON_JSON_H
#include<stdlib.h>


char string_pool[0x8000];
size_t string_cursor = 1;
struct token {
    enum kind {
        UNSET,
        ROOT,
        TRUE,
        FALSE,
        JSON_NULL,
        STRING,
        NUMBER,
        ARRAY,
        OBJECT,
    } kind;
    int root_index;
    void * address;
};
struct token tokens[0x200] = {(struct token) {.kind=ROOT}, (struct token) {.kind=UNSET}};
int token_cursor = 1;

/**
 * Json structures are stored as a flat array of objects holding
 * a link to their parent whose value is their index in that array, index zero being the root
 * Json doesn't require json arrays elements to have an order so sibling data is not stored
 */


static int JSON_TRUE_SINGLETON;
static int JSON_FALSE_SINGLETON;
static int JSON_NULL_SINGLETON;


#define WHITESPACE \
  X(TABULATION, '\t') \
  X(LINE_FEED, '\n') \
  X(CARRIAGE_RETURN, '\r') \
  X(SPACE, ' ')

#define ERRORS \
  X(JSON_ERROR_NO_ERRORS, "No errors found.") \
  X(JSON_ERROR_INVALID_CHARACTER, "Found an unknown token.") \
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
  X(JSON_ERROR_INVALID_CHARACTER_IN_ARRAY, "Invalid character in array.") \
  X(JSON_ERROR_ASSOC_EXPECT_STRING_A_KEY, "A JSON object key must be a quoted string.") \
  X(JSON_ERROR_ASSOC_EXPECT_COLON, "Missing colon after object key.") \
  X(JSON_ERROR_ASSOC_EXPECT_VALUE, "Missing value after JSON object key.")  \
  X(JSON_ERROR_NO_SIBLINGS, "Only Arrays and Objects can have sibling descendants.")  \

#define X(a, b) a,
enum whitespace_tokens { WHITESPACE };
enum json_errors{ ERRORS };
#undef X

#define X(a, b) [a] = b,
char whitespaces[] = {
    WHITESPACE
    '\0',
};

char * json_errors[] = {
    ERRORS
};
#undef X

#undef WHITESPACE
#undef ERRORS

char digit_starters[] = "-0123456789";
char digits[] = "0123456789";
char digits19[] = "123456789";

enum states {
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
    ASSOC_EXPECT_KEY,
    ASSOC_AFTER_KEY_WHITESPACE,
    CLOSE_STRING,
    ASSOC_EXPECT_COLON,
    ASSOC_AFTER_INNER_VALUE
};

struct state {
    enum states kind;
    int ordinal;
    enum json_errors error;
};

int rjson(char*, struct state*, void** );
void print_debug(void);
char * to_string(struct token[0x200], int);

#endif //JSON_JSON_H
