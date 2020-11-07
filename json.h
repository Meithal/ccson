#ifndef JSON_JSON_H
#define JSON_JSON_H
#ifdef WANT_LIBC
#include<stdio.h>
#include<string.h>
#endif


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

#ifndef WANT_LIBC
/* from muslC */
EXPORT size_t cisson_strlen(const char *s)
{
    const char *a = s;
    for (; *s; s++);
    return s-a;
}
#define cs_strlen(s) (cisson_strlen((s)))

/* from muslC */
EXPORT void * cisson_memset(void *dest, int c, size_t n)
{
    unsigned char *s = dest;

    for (; n; n--, s++) *s = c;
    return dest;
}
#define cs_memset(dest, val, repeat) (cisson_memset((dest), (val), (repeat)))

/* from muslC */
EXPORT void *cisson_memcpy(void *restrict dest, const void *restrict src, size_t n)
{
    unsigned char *d = dest;
    const unsigned char *s = src;

    for (; n; n--) *d++ = *s++;
    return dest;
}
#define cs_memcpy(dest, val, repeat) (cisson_memcpy((dest), (val), (repeat)))
#else
#define cs_strlen(s) (strlen((s)))
#define cs_memset(dest, val, repeat) (memset((dest), (val), (repeat)))
#define cs_memcpy(dest, val, repeat) (memcpy((dest), (val), (repeat)))

#endif  /* WANT_LIBC */


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

#ifdef WANT_JSON1
enum json_mode {
    JSON2,
    JSON1,
    RELAXED,
};
#endif

struct state {
    enum states kind;
    size_t ordinal;
    enum json_errors error;
    int root_index;
    unsigned char string_pool[STRING_POOL_SIZE];
    struct token tokens_stack[MAX_TOKENS];
    unsigned char string_cursor;
    int token_cursor;
#ifdef WANT_JSON1
    enum json_mode mode;
#endif
};

/* Parsing */
EXPORT int rjson(unsigned char*, size_t len, struct state*);
/* Output */
EXPORT char* print_debug(struct state * );
EXPORT char* to_string(struct token[0x200], int);
/* Building */
EXPORT void start_string(unsigned char *, const unsigned char [STRING_POOL_SIZE]);
EXPORT void push_string(const unsigned char *, unsigned char [STRING_POOL_SIZE], char* string, int length);
EXPORT void close_root(struct token *, int *);
EXPORT void push_root(int *, const int *);
EXPORT void push_token(enum kind , void * , struct token (*), int * , int);
/* EZ JSON */
#define START_STRING(state_) start_string(&(state_)->string_cursor, (state_)->string_pool)
#define PUSH_STRING(state_, string_, length_) push_string(&(state_)->string_cursor, (state_)->string_pool, (string_), (length_))
#define CLOSE_ROOT(state_) close_root((*state_).tokens_stack, &(*state_).root_index)
#define PUSH_ROOT(state_) push_root(&(state_)->root_index, &(state_)->token_cursor)
#define PUSH_TOKEN(kind_, address_, state_) push_token((kind_), (address_), (state_)->tokens_stack, &(state_)->token_cursor, (state_)->root_index)
#define PUSH_STRING_TOKEN(kind_, state_) PUSH_TOKEN((kind_), (state_)->string_pool + (state_)->string_cursor, (state_))
struct state ez_state__ = {.tokens_stack[0].kind=UNSET};
#define tokenize(string_) ((void)rjson((unsigned char*)(string_), (size_t)cs_strlen(string_), ((void)cs_memset(&ez_state__, 0, sizeof ez_state__), &ez_state__)), (ez_state__).tokens_stack)
#define serialize(paste_) to_string(paste_, (ez_state__).token_cursor)
/* __/ */

#endif //JSON_JSON_H
