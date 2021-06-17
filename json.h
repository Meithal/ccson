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
#define va_(val) val
#else
#define va_(val)
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define c99
#define res restrict
#else
#define res
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
/* from MuslC */
EXPORT size_t cisson_strlen(const unsigned char *s)
{
    const unsigned char *a = s;
    for (; *s; s++);
    return s-a;
}
#define cs_strlen(s) cisson_strlen((const unsigned char*)(s))

/* from MuslC */
EXPORT void * cisson_memset(void *dest, int c, size_t n)
{
    unsigned char *s = dest;

    for (; n; n--, s++) *s = c;
    return dest;
}
#define cs_memset(dest, val, repeat) (cisson_memset((dest), (val), (repeat)))

/* from MuslC */
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
};

struct token {
    enum kind kind;
    int root_index;
    unsigned char * address;
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
  X(JSON_ERROR_UNESCAPED_CONTROL, "Control characters must be escaped.") \

#define X(a, b) a,
enum whitespace_tokens { WHITESPACE };
enum json_errors{ ERRORS };
#undef X

#ifdef c99
#define X(a, b) [a] = b,
#else
#define X(a, b) b,
#endif
static char whitespaces[] = {
    WHITESPACE
    '\0',
};

static char const * json_errors[] = {
    ERRORS
};
#undef X

#undef WHITESPACE
#undef ERRORS

static char digit_starters[] = "-0123456789";
static char digits[] = "0123456789";
static char digits19[] = "123456789";
static char hexdigits[] = "0123456789abcdefABCDEF";

enum states {
    EXPECT_BOM,
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
    ASSOC_EXPECT_KEY,
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

static struct token static_stack[MAX_TOKENS] = { 0 };
static unsigned char static_pool[STRING_POOL_SIZE] = { 0 };

struct cisson_state {
    enum states cur_state;
    int root_index;
    struct tokens {
        struct token *tokens_stack;
        int token_cursor;
    } tokens;
    struct copies {
        unsigned char *string_pool;
        unsigned int string_cursor;
    } copies;
#ifdef WANT_JSON1
    enum json_mode mode;
#endif
};

/* State maintenance */
EXPORT void
start_state(struct cisson_state * state, struct token *stack, size_t stack_size, unsigned char *pool, size_t pool_size);

/* Parsing */
EXPORT enum json_errors rjson(
        size_t len,
        unsigned char cursor[va_(len)],
        struct cisson_state * state);
/* Output */
#ifdef WANT_LIBC
EXPORT char* print_debug(struct tokens * );
#else
#define print_debug(_) ""
#endif
EXPORT unsigned char * res to_string_(struct tokens * res tokens, int compact);
#define to_string(tokens_) (char * res)to_string_(tokens_, 0)
#define to_string_compact(tokens_) (char * res)to_string_(tokens_, 1)
/* Building */
EXPORT void start_string(unsigned int *, const unsigned char [STRING_POOL_SIZE]);
EXPORT void push_string(const unsigned int * res cursor, unsigned char * res pool, char* res string, int length);
EXPORT void close_root(struct token * res, int * res);
EXPORT void push_root(int * res, const int * res);
EXPORT void push_token_kind(enum kind kind, void *res address
                            , struct tokens *tokens, int root_index);
/* EZ JSON */
EXPORT void
push_token(struct cisson_state * state, char token[va_(static 1)]);
EXPORT void
stream_tokens(struct cisson_state * state, char separator, char stream[va_(static 0)], size_t length);
#define START_STRING(state_) start_string(&(state_)->copies.string_cursor, (state_)->copies.string_pool)
#define PUSH_STRING(state_, string_, length_) \
    push_string(                               \
        &(state_)->copies.string_cursor,             \
        (state_)->copies.string_pool,                \
        (string_),                             \
        (length_))
#define CLOSE_ROOT(state_) close_root((*(state_)).tokens.tokens_stack, &(*(state_)).root_index)
#define PUSH_ROOT(state_) push_root(&(state_)->root_index, &(state_)->tokens.token_cursor)
#define PUSH_TOKEN(kind_, address_, state_) \
    push_token_kind(                             \
        (kind_),                            \
        (address_),                         \
        &(state_)->tokens,                  \
        (state_)->root_index)
#define PUSH_STRING_TOKEN(kind_, state_) \
    PUSH_TOKEN((kind_), (state_)->copies.string_pool + (state_)->copies.string_cursor, (state_))
#define START_AND_PUSH_TOKEN(state, kind, string) \
    START_STRING(state);               \
    PUSH_STRING((state), string, (cs_strlen (string))); \
    PUSH_STRING_TOKEN(kind, state)
/* __/ */


#endif //JSON_JSON_H
