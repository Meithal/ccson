/* Automatically generated to be used as a single header replacement
 * of the full library, do not edit. */
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

enum POINTER_ERRORS {
    POINTER_WRONG_SYNTAX = -1,
    POINTER_NOT_FOUND = -2
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


/* Stripped header guard. */

#endif // JSON_JSON_H /* Automatically added header guard. */
#ifdef CISSON_IMPLEMENTATION

static inline size_t
in(char* res hay, unsigned char needle) {
    if (needle == '\0') return 0;
    char * begin = hay;
    for (;*hay; hay++) {
        if(*hay == needle) return (hay - begin) + 1;
    }
    return 0;
}

static inline size_t
len_whitespace(unsigned char * res string) {
    int count = 0;
    while(in(whitespaces, string[count])) {
        count++;
    }
    return count;
}

static inline size_t
remaining(unsigned char * res max, unsigned char * res where) {
    return max - where;
}

EXPORT void
start_string(unsigned int * res cursor,
             const unsigned char pool[STRING_POOL_SIZE]) {
    *cursor += pool[*cursor] + 1;
}

EXPORT void
push_string(const unsigned int *cursor,
            unsigned char *pool,
            char* string,
            int length) {
    cs_memcpy(
            pool + *cursor + 1 + pool[*cursor],
            string,
            length);
    pool[*cursor] += length;
}

EXPORT void
close_root(struct token * tokens,
        int * root_index) {
    *root_index = tokens[*root_index].root_index;
}

EXPORT void
push_root(int * root_index, const int * token_cursor) {
    *root_index = *token_cursor - 1;
}

EXPORT void
push_token_kind(
        enum kind kind,
        void * address,
        struct tokens *tokens,
        int root_index) {
    tokens->tokens_stack[(tokens->token_cursor)++] = (struct token) {
        .kind=kind, .root_index=root_index, .address=address
    };
}

EXPORT void
push_token(struct cisson_state * state, char token[va_(static 1)]) {
    if(token[0] == '>') {
        int i = 0;
        while (token[i] && token[i] == '>') {
            CLOSE_ROOT(state);
            i++;
        }
        return;
    }
    enum kind kind = (enum kind[]){
        UNSET, ROOT, TRUE, FALSE, JSON_NULL, ARRAY, OBJECT,
        NUMBER, NUMBER, NUMBER, NUMBER, NUMBER, NUMBER,
        NUMBER, NUMBER, NUMBER, NUMBER, NUMBER, STRING}
        [in("#tfn[{-0123456789\"", token[0])];
    if(kind != UNSET) {
        START_AND_PUSH_TOKEN(state, kind, token);
    }
    if(in("{[:", token[0])) PUSH_ROOT(state);
}

EXPORT void
stream_tokens(struct cisson_state * state, char separator, char stream[va_(static 0)], size_t length) {
    size_t i = 0;
    while (i < length) {
        size_t token_length = 0;
        while (i + token_length < length && stream[i + token_length] != separator) {
            token_length++;
        }
        stream[i + token_length] = '\0';
        push_token(state, &stream[i]);
        i = i + token_length + sizeof separator;
    }
}

EXPORT void
start_state(
        struct cisson_state * state,
        struct token *stack,
        size_t stack_size,
        unsigned char *pool,
        size_t pool_size) {
    memset(state, 0, sizeof (struct cisson_state));
    memset(stack, 0, stack_size);
    memset(pool, 0, pool_size);
    state->tokens.tokens_stack = stack;
    state->copies.string_pool = pool;
}

EXPORT int
query(struct cisson_state * state, size_t length, char query[va_(length)]) {
    size_t i = 0;
    while (i < length) {
        if (query[i] == '/') {
            size_t token_length = 0;
            while (i + token_length < length && query[i + token_length] != '/') {
                token_length++;
            }
        }
    }

    return 0;
}

EXPORT enum json_errors
rjson(size_t len,
      unsigned char cursor[va_(len)],
      struct cisson_state * external_state) {

#define peek_at(where) cursor[where]
#define SET_STATE_AND_ADVANCE_BY(which_, advance_) \
  state->cur_state = which_; cursor += advance_

    struct cisson_state local_state_ = {0 };
    struct cisson_state * state = &local_state_;
    if(external_state != NULL) {
        state = external_state;
    }
    if (state->tokens.tokens_stack == NULL) {
        memset(static_stack, 0, sizeof(static_stack));
        memset(static_pool, 0, sizeof(static_pool));
        state->tokens.tokens_stack = static_stack;
        state->copies.string_pool = static_pool;
    }

    enum json_errors error = JSON_ERROR_NO_ERRORS;
    unsigned char * final = cursor + len;
    START_AND_PUSH_TOKEN(state, ROOT, "#root");

    // fixme: complete example with self managed memory.
    // todo: fully test reentry
    // todo: make ANSI/STDC compatible
    // todo: complete unicode support?
    // todo: add jasmine mode? aka not copy strings+numbers ?
    // todo: pedantic mode? (forbid duplicate keys, enforce order...)
    // fixme: tokenizer macro functions should be functions
    // todo: test for overflows
    // fixme: check for bounds
    // todo: example with slowly filled array
    // todo: no memory move mode (simple tokenizer)
    // todo: add a dom query function.
    // todo: implement JSON pointer RFC6901
    // todo: implement JSON schema

    for(;;) {
        switch (state->cur_state) {
            case EXPECT_BOM: {
                if(
                        (char)peek_at(0) == '\xEF'
                        && (char)peek_at(1) == '\xBB'
                        && (char)peek_at(2) == '\xBF'
                        ) {
                    SET_STATE_AND_ADVANCE_BY(EXPECT_VALUE, 3);
                } else {
                    SET_STATE_AND_ADVANCE_BY(EXPECT_VALUE, 0);
                }
                break;
            }

            case EXPECT_VALUE:
            {
                if (peek_at(len_whitespace(cursor)+0) == '"') {
                    START_STRING(state);
                    PUSH_STRING(state, (char[]) {peek_at(len_whitespace(cursor) + 0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(FOUND_OPEN_QUOTE, len_whitespace(cursor) + 1);
                }
                else if (peek_at(len_whitespace(cursor) + 0) == '[') {
                    START_AND_PUSH_TOKEN(state, ARRAY, "[");
                    PUSH_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(OPEN_ARRAY, len_whitespace(cursor) + 1);
                }
                else if (peek_at(len_whitespace(cursor) + 0) == '{') {
                    START_AND_PUSH_TOKEN(state, OBJECT, "{");
                    PUSH_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(OPEN_ASSOC, len_whitespace(cursor) + 1);
                }
                else if (in(digit_starters, peek_at(len_whitespace(cursor) + 0))) {
                    START_STRING(state);
                    SET_STATE_AND_ADVANCE_BY(START_NUMBER, len_whitespace(cursor) + 0);
                }
                else if (
                        peek_at(len_whitespace(cursor) + 0) == 't'
                        && peek_at(len_whitespace(cursor) + 1) == 'r'
                        && peek_at(len_whitespace(cursor) + 2) == 'u'
                        && peek_at(len_whitespace(cursor) + 3) == 'e' )
                {
                    START_AND_PUSH_TOKEN(state, TRUE, "true");
                    SET_STATE_AND_ADVANCE_BY(
                            AFTER_VALUE,
                            len_whitespace(cursor)+sizeof("true") - 1)
                            ;
                }
                else if (peek_at(len_whitespace(cursor) + 0) == 'f'
                         && peek_at(len_whitespace(cursor) + 1) == 'a'
                         && peek_at(len_whitespace(cursor) + 2) == 'l'
                         && peek_at(len_whitespace(cursor) + 3) == 's'
                         && peek_at(len_whitespace(cursor) + 4) == 'e') {
                    START_AND_PUSH_TOKEN(state, FALSE, "false");
                    SET_STATE_AND_ADVANCE_BY(
                            AFTER_VALUE,
                            len_whitespace(cursor)+sizeof("false") - 1
                            );
                }
                else if (peek_at(len_whitespace(cursor)+0) == 'n'
                         && peek_at(len_whitespace(cursor)+1) == 'u'
                         && peek_at(len_whitespace(cursor)+2) == 'l'
                         && peek_at(len_whitespace(cursor)+3) == 'l') {
                    START_AND_PUSH_TOKEN(state, JSON_NULL, "null");
                    SET_STATE_AND_ADVANCE_BY(
                            AFTER_VALUE,
                            len_whitespace(cursor)+sizeof("null") - 1
                            );
                }
                else if (state->tokens.tokens_stack[state->root_index].kind != ROOT) {
                    error = JSON_ERROR_ASSOC_EXPECT_VALUE;
                }
                else if (remaining(final, cursor)) {
                    error = JSON_ERROR_INVALID_CHARACTER;
                }
                else {
                    error = JSON_ERROR_EMPTY;
                }

                break;
            }
            case AFTER_VALUE:
            {
                if (state->tokens.tokens_stack[state->root_index].kind == ROOT) {
                    if (remaining(final, cursor + len_whitespace(cursor))) {
                        error = JSON_ERROR_NO_SIBLINGS;
#ifdef WANT_JSON1
                    } else if(state->mode == JSON1 && tokens[state->token_cursor-1].kind != OBJECT) {
                        state->error = JSON_ERROR_JSON1_ONLY_ASSOC_ROOT;
#endif
                    } else {
                        goto exit;
                    }
                }
                else if(state->tokens.tokens_stack[state->root_index].kind == ARRAY) {
                    SET_STATE_AND_ADVANCE_BY(ARRAY_AFTER_VALUE, 0);
                }
                else if(state->tokens.tokens_stack[state->root_index].kind == STRING) {
                    SET_STATE_AND_ADVANCE_BY(ASSOC_AFTER_INNER_VALUE, 0);
                }

                break;
            }
            case OPEN_ARRAY: {
                if (peek_at(len_whitespace(cursor)) == ']') {
                    CLOSE_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(AFTER_VALUE, len_whitespace(cursor) + 1);
                } else {
                    SET_STATE_AND_ADVANCE_BY(EXPECT_VALUE, 0);
                }

                break;
            }
            case OPEN_ASSOC: {
                if (peek_at(len_whitespace(cursor)) == '}') {
                    CLOSE_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(AFTER_VALUE, len_whitespace(cursor) + 1);
                } else {
                    SET_STATE_AND_ADVANCE_BY(ASSOC_EXPECT_KEY, 0);
                }
                break;
            }
            case FOUND_OPEN_QUOTE:
            {
                if (peek_at(0) == '\\') {
                    SET_STATE_AND_ADVANCE_BY(LITERAL_ESCAPE, 1);
                } else if (peek_at(0) == '"') {
                    PUSH_STRING(state, (char[]) {peek_at(0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(CLOSE_STRING, 1);
                } else {
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 0);
                }

                break;
            }
            case LITERAL_ESCAPE: {
                error = JSON_ERROR_INVALID_ESCAPE_SEQUENCE * !in("\\\"/u", peek_at(0));  /* u"\/ */
                if (in("\\\"/", peek_at(0))) {
                    PUSH_STRING(state, (char[]) {peek_at(0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 1);
                }
                else if (peek_at(0) == 'u') {
                    if(!(in(hexdigits, peek_at(1))
                    && in(hexdigits, peek_at(2))
                    && in(hexdigits, peek_at(3))
                    && in(hexdigits, peek_at(4)))
                    ) {
                        error = JSON_ERROR_INCOMPLETE_UNICODE_ESCAPE;
                        break;
                    }
                    PUSH_STRING(state, (char*)cursor, 5);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 5);
                    break;
                }
                else {
                    error = JSON_ERROR_INVALID_ESCAPE_SEQUENCE;
                }

                break;
            }
            case IN_STRING: {
                error = JSON_ERROR_UNESCAPED_CONTROL * (peek_at(0) < 0x20 && !in("\b\f\n\r\t", peek_at(0)));
                PUSH_STRING(state, (char[]) {peek_at(0)}, error == JSON_ERROR_NO_ERRORS);
                SET_STATE_AND_ADVANCE_BY(
                        (
                                (int[]){
                                        IN_STRING,
                                        LITERAL_ESCAPE,
                                        CLOSE_STRING
                                }[in("\\\"", peek_at(0))]),
                        /*state->error == JSON_ERROR_NO_ERRORS);*/
                        1);

                break;
            }

            case START_NUMBER: {
                PUSH_STRING(
                        state,
                ((char[]){peek_at(0)}),
                ((int[]){0, 1}[in("-", peek_at(0))])
                    );
                SET_STATE_AND_ADVANCE_BY(
                    NUMBER_AFTER_MINUS,
                    (
                            (int[]){0, 1}[in("-", peek_at(0))])
                    );

                break;
            }

            case NUMBER_AFTER_MINUS:
            {
                if (peek_at(0) == '0') {
                    PUSH_STRING(state, (char[]){peek_at(0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(EXPECT_FRACTION, 1);
                } else if (in(digits19, peek_at(0))) {
                    PUSH_STRING(state, (char[]){peek_at(0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(IN_NUMBER, 1);
                } else {
                    error = JSON_ERROR_INVALID_NUMBER;
                }

                break;
            }
            case IN_NUMBER: {
                PUSH_STRING(
                        state,
                        (char[]){peek_at(0)},
                        (in(digits, peek_at(0)) != 0)
                );
                SET_STATE_AND_ADVANCE_BY(
                        ((int[]){
                            EXPECT_FRACTION,
                            IN_NUMBER}[in(digits, peek_at(0)) != 0]),
                (in(digits, peek_at(0)) != 0)
                );

                break;
            }
            case EXPECT_FRACTION: {
                PUSH_STRING(
                        state,
                        (char[]){peek_at(0)},
                        (peek_at(0) == '.')
                );
                SET_STATE_AND_ADVANCE_BY(
                        ((int[]){
                                EXPECT_EXPONENT,
                                IN_FRACTION}[peek_at(0) == '.']),
                        (peek_at(0) == '.')
                );

                break;
            }

            case EXPECT_EXPONENT: {
                if (peek_at(0) == 'e' || peek_at(0) == 'E') {
                    PUSH_STRING(state, (char[]){peek_at(0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(EXPONENT_EXPECT_PLUS_MINUS, 1);
                } else {
                    PUSH_STRING_TOKEN(NUMBER, state);
                    SET_STATE_AND_ADVANCE_BY(AFTER_VALUE, 0);
                }

                break;
            }

            case IN_FRACTION: {
                error = JSON_ERROR_INVALID_NUMBER * (in(digits, peek_at(0)) == 0);
                PUSH_STRING(state,
                            (char[]){peek_at(0)},
                            error == JSON_ERROR_NO_ERRORS);
                SET_STATE_AND_ADVANCE_BY(
                        ((int[]){
                            IN_FRACTION,
                            IN_FRACTION_DIGIT
                        }[error == JSON_ERROR_NO_ERRORS]),
                        error == JSON_ERROR_NO_ERRORS);

                break;
            }

            case IN_FRACTION_DIGIT: {
                if (in(digits, peek_at(0))) {
                    PUSH_STRING(state, (char[]){peek_at(0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(IN_FRACTION_DIGIT, 1);
                } else {
                    SET_STATE_AND_ADVANCE_BY(EXPECT_EXPONENT, 0);
                }

                break;
            }

            case EXPONENT_EXPECT_PLUS_MINUS: {
                if (peek_at(0) == '+' || peek_at(0) == '-') {
                    PUSH_STRING(state, (char[]){peek_at(0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(EXPECT_EXPONENT_DIGIT, 1);
                } else {
                    SET_STATE_AND_ADVANCE_BY(EXPECT_EXPONENT_DIGIT, 0);
                }

                break;
            }

            case EXPECT_EXPONENT_DIGIT: {
                if (in(digits, peek_at(0))) {
                    PUSH_STRING(state, (char[]){peek_at(0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(IN_EXPONENT_DIGIT, 1);
                } else {
                    error = JSON_ERROR_INVALID_NUMBER;
                }

                break;
            }

            case IN_EXPONENT_DIGIT: {
                if (in(digits, peek_at(0))) {
                    PUSH_STRING(state, (char[]) {peek_at(0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(IN_EXPONENT_DIGIT, 1);
                } else {
                    PUSH_STRING_TOKEN(NUMBER, state);
                    SET_STATE_AND_ADVANCE_BY(AFTER_VALUE, len_whitespace(cursor) + 0);
                }

                break;
            }

            case ARRAY_AFTER_VALUE: {
                if(peek_at(len_whitespace(cursor)) == ',') {
                    SET_STATE_AND_ADVANCE_BY(EXPECT_VALUE, len_whitespace(cursor) + 1);
                } else if(peek_at(len_whitespace(cursor) + 0) == ']') {
                    CLOSE_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(AFTER_VALUE, len_whitespace(cursor) + 1);
                } else {
                    error = JSON_ERROR_INVALID_CHARACTER_IN_ARRAY;
                }

                break;
            }

            case ASSOC_EXPECT_KEY: {
                if(peek_at(len_whitespace(cursor) + 0) == '"') {
                    START_STRING(state);
                    PUSH_STRING(state, (char[]) {peek_at(len_whitespace(cursor) + 0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, len_whitespace(cursor) + 1);
                } else {
                    error = JSON_ERROR_ASSOC_EXPECT_STRING_A_KEY;
                }
                break;
            }

            case CLOSE_STRING: {  /* fixme: non advancing state */
                PUSH_STRING_TOKEN(STRING, state);
                if ((state->tokens.tokens_stack)[state->root_index].kind == OBJECT) {
                    PUSH_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(ASSOC_EXPECT_COLON, 0);
                } else {
                    SET_STATE_AND_ADVANCE_BY(AFTER_VALUE, 0);
                }

                break;
            }

            case ASSOC_EXPECT_COLON: {
                error = JSON_ERROR_ASSOC_EXPECT_COLON * (peek_at(len_whitespace(cursor)) != ':');
                SET_STATE_AND_ADVANCE_BY(
                        ((int[]){
                            ASSOC_EXPECT_COLON,
                            EXPECT_VALUE}[error == JSON_ERROR_NO_ERRORS]),
                (len_whitespace(cursor) + 1) * (error == JSON_ERROR_NO_ERRORS));
                break;
            }

            case ASSOC_AFTER_INNER_VALUE: {
                if(peek_at(len_whitespace(cursor)) == ',') {
                    CLOSE_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(ASSOC_EXPECT_KEY, len_whitespace(cursor) + 1);
                } else if (peek_at(len_whitespace(cursor)) == '}'){
                    CLOSE_ROOT(state);
                    CLOSE_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(AFTER_VALUE, len_whitespace(cursor) +1);
                } else {
                    error = JSON_ERROR_ASSOC_EXPECT_VALUE;
                }

                break;
            }
        }

        if(error != JSON_ERROR_NO_ERRORS) {
            goto exit;
        }
    }

    exit: return error;
#undef peek_at
#undef SET_STATE_AND_ADVANCE_BY
}

#ifdef WANT_LIBC
static char output[STRING_POOL_SIZE];
EXPORT char* print_debug(struct tokens * tokens) {
    int j;
    int cursor = 0;
    struct token *stack = tokens->tokens_stack;

    memset(output, 0, sizeof(output));
    for (j = 0; j < tokens->token_cursor; ++j) {
        char dest[STRING_POOL_SIZE] = {0};

        cursor += snprintf(
            output + cursor, 80,
            "%d: kind: %s, root: %d, value: %s\n", j,
            (char*[]){
                    "UNSET", "ROOT", "TRUE", "FALSE", "JSON_NULL",
                    "STRING", "NUMBER", "ARRAY", "OBJECT"
            }[stack[j].kind],
            stack[j].root_index,
            (char*)memcpy(
                    dest,
                    (char*)(stack[j].address)+1,
                    *((char*)stack[j].address)
                    )
        );
    }

    return output;
}
#endif

static char ident_s[0x80];
static char * print_ident(int ident, unsigned compact) {
    cs_memset(ident_s, ' ', ident * 2 * (compact ^ 1u));
    ident_s[ident * 2 * (compact ^ 1u)] = '\0';
    return ident_s;
}

EXPORT size_t minified_string(unsigned char * target, const unsigned char * source, int bytecount) {
    if(!bytecount) return 0;
    int i;
    int inc;
    unsigned char n;
    unsigned char * start = target;
    for(i = 0; i < bytecount ; i+=inc) {
        n = source[i];
        inc = 1;
        if(n == '\\') {
            if(i == bytecount - 1) return *(target++) = '*', *(target++) = '*', *(target) = '*', -1;  /*
            * escape sequence at the end of a token */
            if(source[i + 1] == '/') *(target++) = '/';
            else if (in("\\\"bfnrtu",source[i + 1])) *(target++) = '\\', *(target++) = source[i + 1];
            /* fixme: \uXXXX are longer than their utf8 equivalent. */
            else return *(target++) = '*', *(target++) = '*', *(target) = '*', -2;  /* invalid escape sequence */
            inc = 2;
        }
        else if (n == '\b') *(target++) = '\\', *(target++) = 'b';
        else if (n == '\f') *(target++) = '\\', *(target++) = 'f';
        else if (n == '\n') *(target++) = '\\', *(target++) = 'n';
        else if (n == '\r') *(target++) = '\\', *(target++) = 'r';
        else if (n == '\t') *(target++) = '\\', *(target++) = 't';
        else if (n < ' ') {  /* ascii control characters */
            *(target++) = '\\', *(target++) = 'u', *(target++) = '0', *(target++) = '0';
            *(target++) = hexdigits[n/16], *(target++) = hexdigits[n%16];
        }
        else if(n > 0xFFFF / 24) {  /* fixme: check for valid unicode, especially for low codepoints and >= 0x10000 */
            if( i == (bytecount - 1) && (n & 0x80u) != 0x80 /* beginning of a unicode sequence at the end of the string */
            || (i > 0 && i < (bytecount - 1) && (n & 0x80u) == 0x80 && (source[i + 1] & 0x80u) != 0x80 && (source[i - 1] & 0x80u) != 0x80)/* isolated high order bytes */
            )
                *(target++) = '\xff', *(target++) = '\xfd';
            else
                *(target++) = n;  /* paste raw utf8 as is,
                   * fixme: convert utf16 surrogates into utf8,
                   * anything \u encoded into utf8
                   * +decode codepoint and find shortest utf8 encoding.
                   * unsigned char decoded = (string[state->ordinal]&0x1Fu<<5u)|(string[state->ordinal+1]&0x7Fu);
                   */
        } else {
            *(target++) = n;
        }
    }

    return target - start;
}

EXPORT unsigned char * res
to_string_(struct tokens * res tokens, int compact) {
    // todo: make the caller handle the buffer
    struct token *stack = tokens->tokens_stack;
    int max = tokens->token_cursor;

#define cat(where, string, token) (\
    minified_string((where), (string), *((char*)(token)->address)\
))
#define cat_raw(where, string) (cs_memcpy((where), (string), cs_strlen((string))), cs_strlen((string)))

    typedef unsigned char u8;

    static unsigned char output[STRING_POOL_SIZE];
    cs_memset(output, 0, sizeof output);
    size_t cursor = 0;
    int ident = 0;
    int j;
    for (j = 1; j < max; ++j) {

        if(stack[j].kind == UNSET) {
            continue;
        }

        if (stack[stack[j].root_index].kind == STRING) {
            cursor += cat_raw(output + cursor, compact ? ":" : ": ");
        } else {
            cursor += cat_raw(output + cursor, print_ident(ident, compact));
        }

        unsigned char dest[STRING_POOL_SIZE] = {0};
        cs_memcpy(dest, (char*)stack[j].address + 1, *((char *) stack[j].address));
        cursor += cat(output + cursor, dest, &stack[j]);

        if(stack[j].kind == ARRAY || stack[j].kind == OBJECT) {
            cursor += cat_raw(output + cursor, compact ? "" : "\n");
            ident++;
        }
        if (j <= max) {
            if (stack[j + 1].root_index < stack[j].root_index) {
                if (j + 1 == max
                || stack[j + 1].root_index != stack[stack[j].root_index].root_index
                || stack[stack[j + 1].root_index].kind == ARRAY) {
                    int target = stack[j + 1].root_index;
                    int cur_node = j;
                    for (;;) {

                        if(stack[stack[cur_node].root_index].kind == ARRAY) {
                            cursor += cat_raw(output + cursor, compact ? "" : "\n");
                            --ident;
                            cursor += cat_raw(output + cursor, print_ident(ident, compact));
                            cursor += cat_raw(output + cursor, "]");
                        }
                        else if(stack[stack[cur_node].root_index].kind == OBJECT) {
                            cursor += cat_raw(output + cursor, compact ? "" : "\n");
                            --ident;
                            cursor += cat_raw(output + cursor, print_ident(ident, compact));
                            cursor += cat_raw(output + cursor, "}");
                        }
                        if(stack[(cur_node = stack[cur_node].root_index)].root_index == target) {
                            break;
                        }
                    }
                }
            }
            if (j + 1 < max && (
               stack[stack[j].root_index].kind == STRING || stack[stack[j].root_index].kind == ARRAY
            ) && stack[j].kind != ARRAY && stack[j].kind != OBJECT) {
                cursor += cat_raw(output + cursor, compact ? "," : ",\n");
            }
        }
    }

    return output;
#undef cat
#undef cat_raw
}

#endif  // CISSON_IMPLEMENTATION