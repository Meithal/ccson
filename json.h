#ifndef JSON_JSON_H
#define JSON_JSON_H
#include<string.h>
#include<stdio.h>


#if (!(defined(_WIN32) || defined(_WIN64)) \
|| defined(__CYGWIN__) \
|| defined(__MINGW32__) \
|| defined(__MINGW64__))
#define HAS_VLA
#endif

#ifdef HAS_VLA
#define EXPORT extern
#else
#define EXPORT __declspec(dllexport)
#endif

#ifndef NULL  /* No libC */
#define size_t unsigned long long
#define NULL ((void *)0)
#endif

/*****************************************************/


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
#ifndef SELF_MANAGE_MEMORY
unsigned char string_pool__[STRING_POOL_SIZE];
struct token tokens__[MAX_TOKENS] = {{.kind=UNSET}};
#endif
#ifndef STRING_POOL_SIZE
#define STRING_POOL_SIZE 0x2000
#endif
#ifndef MAX_TOKENS
#define MAX_TOKENS 0x200
#endif

/**
 * Json structures are stored as a flat array of objects holding
 * a link to their parent whose value is their index in that array, index zero being the root
 * Json doesn't require json arrays elements to have an order so sibling data is not stored.
 */

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
  X(JSON_ERROR_INVALID_ESCAPE_SEQUENCE, "Invalid escape sequence.")                       \
  X(JSON_ERROR_INVALID_NUMBER, "Malformed number.")              \
  X(JSON_ERROR_INVALID_CHARACTER_IN_ARRAY, "Invalid character in array.") \
  X(JSON_ERROR_ASSOC_EXPECT_STRING_A_KEY, "A JSON object key must be a quoted string.") \
  X(JSON_ERROR_ASSOC_EXPECT_COLON, "Missing colon after object key.") \
  X(JSON_ERROR_ASSOC_EXPECT_VALUE, "Missing value after JSON object key.")  \
  X(JSON_ERROR_NO_SIBLINGS, "Only Arrays and Objects can have sibling descendants.")      \
  X(JSON_ERROR_JSON1_ONLY_ASSOC_ROOT, "JSON1 only allows objects as root element.")  \
  X(JSON_ERROR_UTF16_NOT_SUPPORTED_YET, "Code points greater than 0x0800 not supported yet.") \
  X(JSON_ERROR_INCOMPLETE_UNICODE_ESCAPE, "Incomplete unicode character sequence.") \

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
char hexdigits[] = "0123456789abcdefABCDEF";

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

char num_to_hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

#ifdef WANT_JSON1
enum json_mode {
    JSON2,
    JSON1,
    RELAXED,
};
#endif

struct state {
    enum states kind;
    int ordinal;
    enum json_errors error;
    int root_index;
    unsigned char (*string_pool)[STRING_POOL_SIZE];
    struct token (*tokens_stack)[MAX_TOKENS];
    int string_cursor;
    int token_cursor;
#ifdef WANT_JSON1
    enum json_mode mode;
#endif
};

/* Parsing */
EXPORT int rjson(unsigned char*, size_t len, struct state*);
/* Output */
EXPORT char* print_debug(struct state * state);
EXPORT char* to_string(struct token[0x200], int);
/* Building */
EXPORT void start_string(struct state * state);
EXPORT void push_string(struct state* state, char* string, int length);
EXPORT void close_root(struct state * state);
EXPORT void push_root(struct state * state);
EXPORT void push_token(enum kind kind, void * address, struct state * state);
/* __/ */

#endif //JSON_JSON_H
