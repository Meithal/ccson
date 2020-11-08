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
EXPORT size_t cisson_strlen(const unsigned char *s)
{
    const unsigned char *a = s;
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
    unsigned int string_cursor;
    int token_cursor;
#ifdef WANT_JSON1
    enum json_mode mode;
#endif
};

/* Parsing */
EXPORT int rjson(unsigned char*, size_t len, struct state*);
/* Output */
#ifdef WANT_LIBC
EXPORT char* print_debug(struct state * );
#else
#define print_debug(_) ""
#endif
EXPORT unsigned char *to_string_(struct token tokens_[MAX_TOKENS], int max, int compact);
#define to_string(tokens_, max) (char *)to_string_(tokens_, max, 0)
#define to_string_compact(tokens_, max) (char *)to_string_(tokens_, max, 1)
/* Building */
EXPORT void start_string(unsigned int *, const unsigned char [STRING_POOL_SIZE]);
EXPORT void push_string(const unsigned int *cursor, unsigned char *pool, char* string, int length);
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
#define START_AND_PUSH_TOKEN(state, kind, string) START_STRING(state); PUSH_STRING(state, string, (sizeof string) - 1); PUSH_STRING_TOKEN(kind, state)
/* __/ */

/* Stripped header guard. */


static inline int in(char* hay, unsigned char needle) {
    if (needle == '\0') return 0;
    for (;*hay; hay++) {
        if(*hay == needle) return 1;
    }
    return 0;
}

static inline size_t len_whitespace(unsigned char* string, size_t ordinal) {
    int count = 0;
    while(in(whitespaces, string[ordinal + count])) {
        count++;
    }
    return count;
}

static inline size_t remaining(size_t max, size_t where) {
    return max - where;
}

EXPORT void start_string(unsigned int * cursor, const unsigned char pool[STRING_POOL_SIZE]) {
    *cursor += pool[*cursor] + 1;
}

EXPORT void push_string(const unsigned int *cursor, unsigned char *pool, char* string, int length) {
    cs_memcpy(pool + *cursor + 1 + pool[*cursor], string, length);
    pool[*cursor] += length;
}

EXPORT void close_root(struct token * tokens, int * root_index) {
    *root_index = tokens[*root_index].root_index;
}

EXPORT void push_root(int * root_index, const int * token_cursor) {
    *root_index = *token_cursor - 1;
}

EXPORT void push_token(enum kind kind, void * address, struct token *tokens, int * token_cursor, int root_index) {
    tokens[(*token_cursor)++] = (struct token) {.kind=kind, .root_index=root_index, .address=address};
}

EXPORT int rjson(unsigned char* string, size_t len, struct state* state) {

#define peek_at(where) string[state->ordinal + where]
#define SET_STATE_AND_ADVANCE_BY(which_, advance_) state->kind = which_; state->ordinal += advance_
    struct token (*tokens)[MAX_TOKENS] = &state->tokens_stack;
    START_AND_PUSH_TOKEN(state, ROOT, "#root");

    // fixme: complete example with self managed memory.
    // todo: fully test reentrance
    // todo: make ANSI/STDC compatible
    // todo: complete unicode support?
    // todo: add jasmine mode? aka not copy strings+numbers ?
    // fixme: Transform control characters and escape sequence only when displaying, helps compat with jasmine

    if (state->ordinal == 0) {
        state->error = JSON_ERROR_NO_ERRORS;
        state->kind = WHITESPACE_BEFORE_VALUE;
    }

    for(;;) {
        switch (state->kind) {
            case WHITESPACE_BEFORE_VALUE: {
                SET_STATE_AND_ADVANCE_BY(EXPECT_VALUE, len_whitespace(string, state->ordinal));
                break;
            }

            case WHITESPACE_AFTER_VALUE: {
                SET_STATE_AND_ADVANCE_BY(AFTER_VALUE, len_whitespace(string, state->ordinal));
                break;
            }

            case EXPECT_VALUE:
            {
                if (peek_at(0) == '"') {
                    START_STRING(state);
                    PUSH_STRING(state, "\"", 1);
                    SET_STATE_AND_ADVANCE_BY(FOUND_OPEN_QUOTE, 1);
                }
                else if (peek_at(0) == '[') {
                    START_AND_PUSH_TOKEN(state, ARRAY, "[");
                    PUSH_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(OPEN_ARRAY, 1);
                }
                else if (peek_at(0) == '{') {
                    START_AND_PUSH_TOKEN(state, OBJECT, "{");
                    PUSH_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(OPEN_ASSOC, 1);
                }
                else if (in(digit_starters, peek_at(0))) {
                    START_STRING(state);
                    SET_STATE_AND_ADVANCE_BY(START_NUMBER, 0);
                }
                else if (
                        peek_at(0) == 't'
                        && peek_at(1) == 'r'
                        && peek_at(2) == 'u'
                        && peek_at(3) == 'e' )
                {
                    START_AND_PUSH_TOKEN(state, TRUE, "true");
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_AFTER_VALUE, sizeof("true") - 1);
                }
                else if (peek_at(0) == 'f'
                         && peek_at(1) == 'a'
                         && peek_at(2) == 'l'
                         && peek_at(3) == 's'
                         && peek_at(4) == 'e') {
                    START_AND_PUSH_TOKEN(state, FALSE, "false");
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_AFTER_VALUE, sizeof("false") - 1);
                }
                else if (peek_at(0) == 'n'
                         && peek_at(1) == 'u'
                         && peek_at(2) == 'l'
                         && peek_at(3) == 'l') {
                    START_AND_PUSH_TOKEN(state, JSON_NULL, "null");
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_AFTER_VALUE, sizeof("null") - 1);
                }
                else if (tokens[state->root_index]->kind != ROOT) {
                    state->error = JSON_ERROR_ASSOC_EXPECT_VALUE;
                }
                else if (remaining(len, state->ordinal)) {
                    state->error = JSON_ERROR_INVALID_CHARACTER;
                }
                else {
                    state->error = JSON_ERROR_EMPTY;
                }

                break;
            }
            case AFTER_VALUE:
            {
                if (state->tokens_stack[state->root_index].kind == ROOT) {
                    if (remaining(len, state->ordinal)) {
                        state->error = JSON_ERROR_NO_SIBLINGS;
#ifdef WANT_JSON1
                    } else if(state->mode == JSON1 && tokens[state->token_cursor-1].kind != OBJECT) {
                        state->error = JSON_ERROR_JSON1_ONLY_ASSOC_ROOT;
#endif
                    } else {
                        return state->token_cursor;
                    }
                }
                else if(state->tokens_stack[state->root_index].kind == ARRAY) {
                    SET_STATE_AND_ADVANCE_BY(ARRAY_AFTER_VALUE, 0);
                }
                else if(state->tokens_stack[state->root_index].kind == OBJECT) {
                    SET_STATE_AND_ADVANCE_BY(ASSOC_AFTER_VALUE, 0);
                }
                else if(state->tokens_stack[state->root_index].kind == STRING) {
                    SET_STATE_AND_ADVANCE_BY(ASSOC_AFTER_INNER_VALUE, 0);
                }

                break;
            }
            case OPEN_ARRAY: {
                if (peek_at(len_whitespace(string, state->ordinal)) == ']') {
                    CLOSE_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_AFTER_VALUE, len_whitespace(string, state->ordinal) + 1);
                } else {
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_BEFORE_VALUE, 0);
                }

                break;
            }
            case OPEN_ASSOC: {
                if (peek_at(len_whitespace(string, state->ordinal)) == '}') {
                    CLOSE_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_AFTER_VALUE, len_whitespace(string, state->ordinal) + 1);
                } else {
                    SET_STATE_AND_ADVANCE_BY(ASSOC_WHITESPACE_BEFORE_KEY, 0);
                }
                break;
            }
            case FOUND_OPEN_QUOTE:
            {
                if (peek_at(0) == '\\') {
                    SET_STATE_AND_ADVANCE_BY(LITERAL_ESCAPE, 1);
                } else if (peek_at(0) == '"') {
                    PUSH_STRING(state, "\"", 1);
                    SET_STATE_AND_ADVANCE_BY(CLOSE_STRING, 1);
                } else {
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 0);
                }

                break;
            }
            case LITERAL_ESCAPE: {
                if (peek_at(0) == '\\') {
                    PUSH_STRING(state, "\\\\", 2);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 1);
                }
                else if (peek_at(0) == '"') {
                    PUSH_STRING(state, "\\\"", 2);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 1);
                }
                else if (peek_at(0) == '/') {
                    PUSH_STRING(state, "/", 1);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 1);
                }
                else if (peek_at(0) == 'u') {
                    if(!(in(hexdigits, peek_at(1))
                    && in(hexdigits, peek_at(2))
                    && in(hexdigits, peek_at(3))
                    && in(hexdigits, peek_at(4)))
                    ) {
                        state->error = JSON_ERROR_INCOMPLETE_UNICODE_ESCAPE;
                        break;
                    }
                    PUSH_STRING(state, "\\u", 2);
                    PUSH_STRING(state, (char*)string+state->ordinal+1, 4);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 5);
                    break;
                }
                else {
                    state->error = JSON_ERROR_INVALID_ESCAPE_SEQUENCE;
                }

                break;
            }
            case IN_STRING: {
                if (peek_at(0) == '\\')
                {
                    SET_STATE_AND_ADVANCE_BY(LITERAL_ESCAPE, 1);
                }
                else if (peek_at(0) == '"')
                {
                    PUSH_STRING(state, "\"", 1);
                    SET_STATE_AND_ADVANCE_BY(CLOSE_STRING, 1);
                }
                else {
                    PUSH_STRING(state, (char[]){peek_at(0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 1);
                }

                break;
            }
            case START_NUMBER: {
                if (peek_at(0) == '-') {
                    PUSH_STRING(state, (char[]){peek_at(0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(NUMBER_AFTER_MINUS, 1);
                } else {
                    SET_STATE_AND_ADVANCE_BY(NUMBER_AFTER_MINUS, 0);
                }

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
                    state->error = JSON_ERROR_INVALID_NUMBER;
                }

                break;
            }
            case IN_NUMBER: {
                if (in(digits, peek_at(0))) {
                    PUSH_STRING(state, (char[]){peek_at(0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(IN_NUMBER, 1);
                } else {
                    SET_STATE_AND_ADVANCE_BY(EXPECT_FRACTION, 0);
                }

                break;
            }
            case EXPECT_FRACTION: {
                if (peek_at(0) == '.') {
                    PUSH_STRING(state, (char[]){peek_at(0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(IN_FRACTION, 1);
                } else {
                    SET_STATE_AND_ADVANCE_BY(EXPECT_EXPONENT, 0);
                }

                break;
            }
            case EXPECT_EXPONENT: {
                if (peek_at(0) == 'e' || peek_at(0) == 'E') {
                    PUSH_STRING(state, (char[]){peek_at(0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(EXPONENT_EXPECT_PLUS_MINUS, 1);
                } else {
                    PUSH_STRING_TOKEN(NUMBER, state);
//                    push_token(((NUMBER)), ((state)->string_pool + (state)->string_cursor), ((state))->tokens_stack,
//                               &((state))->token_cursor, ((state))->root_index);
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_AFTER_VALUE, 0);
                }

                break;
            }
            case IN_FRACTION: {
                if (in(digits, peek_at(0))) {
                    PUSH_STRING(state, (char[]){peek_at(0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(IN_FRACTION_DIGIT, 1);
                } else {
                    state->error = JSON_ERROR_INVALID_NUMBER;
                }

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
                    state->error = JSON_ERROR_INVALID_NUMBER;
                }

                break;
            }
            case IN_EXPONENT_DIGIT: {
                if (in(digits, peek_at(0))) {
                    PUSH_STRING(state, (char[]) {peek_at(0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(IN_EXPONENT_DIGIT, 1);
                } else {
                    PUSH_STRING_TOKEN(NUMBER, state);
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_AFTER_VALUE, 0);
                }

                break;
            }
            case ARRAY_AFTER_VALUE: {
                if(peek_at(0) == ',') {
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_BEFORE_VALUE, 1);
                } else if(peek_at(0) == ']') {
                    CLOSE_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_AFTER_VALUE, 1);
                } else {
                    state->error = JSON_ERROR_INVALID_CHARACTER_IN_ARRAY;
                }

                break;
            }
            case ASSOC_AFTER_VALUE: {
                CLOSE_ROOT(state);
                SET_STATE_AND_ADVANCE_BY(WHITESPACE_AFTER_VALUE, 0);

                break;
            }
            case ASSOC_WHITESPACE_BEFORE_KEY: {
                SET_STATE_AND_ADVANCE_BY(ASSOC_EXPECT_KEY, len_whitespace(string, state->ordinal));

                break;
            }
            case ASSOC_EXPECT_KEY: {
                if(peek_at(0) == '"') {
                    START_STRING(state);
                    PUSH_STRING(state, "\"", 1);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 1);
                } else {
                    state->error = JSON_ERROR_ASSOC_EXPECT_STRING_A_KEY;
                }
                break;
            }
            case ASSOC_AFTER_KEY_WHITESPACE: {
                SET_STATE_AND_ADVANCE_BY(ASSOC_EXPECT_COLON, len_whitespace(string, state->ordinal));

                break;
            }
            case CLOSE_STRING: {
                PUSH_STRING_TOKEN(STRING, state);
                if ((*tokens)[state->root_index].kind == OBJECT) {
                    PUSH_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(ASSOC_AFTER_KEY_WHITESPACE, 0);
                } else {
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_AFTER_VALUE, 0);
                }

                break;
            }
            case ASSOC_EXPECT_COLON: {
                if(peek_at(0) == ':') {
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_BEFORE_VALUE, 1);
                } else {
                    state->error = JSON_ERROR_ASSOC_EXPECT_COLON;
                }
                break;
            }
            case ASSOC_AFTER_INNER_VALUE: {
                if(peek_at(0) == ',') {
                    CLOSE_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(ASSOC_WHITESPACE_BEFORE_KEY, 1);
                } else if (peek_at(0) == '}'){
                    CLOSE_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(ASSOC_AFTER_VALUE, 1);
                } else {
                    state->error = JSON_ERROR_ASSOC_EXPECT_VALUE;
                }
                break;
            }
        }

        if(state->error != JSON_ERROR_NO_ERRORS) {
            return state->token_cursor;
        }
    }
#undef peek_at
#undef SET_STATE_AND_ADVANCE_BY
}

#ifdef WANT_LIBC
EXPORT char* print_debug(struct state * state) {
    int j;
    int cursor = 0;
    struct token *tokens = state->tokens_stack;

    static char output[STRING_POOL_SIZE];
    for (j = 0; j < state->token_cursor; ++j) {
        cursor += snprintf(output, 80, "%d: kind: %s, root: %d", j, (char*[]){
                "UNSET", "ROOT", "TRUE", "FALSE", "JSON_NULL",
                "STRING", "NUMBER", "ARRAY", "OBJECT"
        }[tokens[j].kind],
                tokens[j].root_index);
        if(tokens[j].kind == STRING || tokens[j].kind == NUMBER) {
            char dest[STRING_POOL_SIZE] = {0};
            cursor += snprintf(output + cursor, 80,", value: %s",
                   (char*)memcpy(dest, (char*)(tokens[j].address)+1, *((char*)tokens[j].address))
                   );
        }
    }

    output[cursor] = '\0';
    return output;
}
#endif

static unsigned char ident_s[0x80];
static unsigned char * print_ident(int ident, unsigned compact) {
    cs_memset(ident_s, ' ', ident * 2 * (compact ^ 1u));
    ident_s[ident * 2 * (compact ^ 1u)] = '\0';
    return ident_s;
}

/**
 * Turns \0 into \u0000, \/ into / , this makes json valid
 * but also helps to compare two json objects for equality.
 * */
EXPORT size_t shortest_safe_string(unsigned char * target, const unsigned char * source, int bytecount) {
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
        else if(n > 0xFF / 2) {  /* fixme: check for valid unicode, especially for low codepoints and >= 0x10000 */
            /* todo: extended ascii support can be added here */
            if(i == bytecount - 1 && (n & 0x80u) != 0x80) return *(target++) = '*', *(target++) = '*', *(target) = '*', -3;
                    /* beginning of a unicode sequence at the end of the string */
            *(target++) = n;  /* paste raw utf8 as is, fixme: convert utf16 surrogates into utf8,
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

static inline size_t get_length(struct token *token) {
    return (int[]){
        [UNSET] = 0, [ROOT] = 0,
        [TRUE] = sizeof("true") - 1,
        [FALSE] = sizeof("false") - 1,
        [JSON_NULL] = sizeof("null") - 1,
        [STRING] = *((char*)token->address),
        [NUMBER] = *((char*)token->address),
        [ARRAY] = 1, [OBJECT] = 1
    }[token->kind];
}

EXPORT unsigned char *to_string_(struct token tokens[MAX_TOKENS], int max, int compact) {
    // todo: make the caller handle the buffer

#define cat(where, string, token) (\
    shortest_safe_string((where), (string), get_length((token))\
))
#define cat_raw(where, string) (cs_memcpy((where), (string), cs_strlen((string))), cs_strlen((string)))

    typedef unsigned char u8;

    static unsigned char output[STRING_POOL_SIZE];
    cs_memset(output, 0, sizeof output);
    size_t cursor = 0;
    int ident = 0;
    int j;
    for (j = 1; j < max; ++j) {
        if (tokens[tokens[j].root_index].kind == STRING) {
            cursor += cat_raw(output + cursor, compact ? ":" : ": ");
        } else {
            cursor += (cs_memcpy(output + cursor, print_ident(ident, compact), cs_strlen(print_ident(ident, compact))), cs_strlen(print_ident(ident, compact)));
        }

        unsigned char dest[STRING_POOL_SIZE] = {0};
        cs_memcpy(dest, (char*)tokens[j].address + 1, *((char *) tokens[j].address));
        cursor += cat(output + cursor, dest, &tokens[j]);

        if(tokens[j].kind == ARRAY || tokens[j].kind == OBJECT) {
            cursor += cat_raw(output + cursor, compact ? "" : "\n");
            ident++;
        }
        if (j <= max) {
            if (tokens[j + 1].root_index < tokens[j].root_index) {
                if (j + 1 == max
                || tokens[j + 1].root_index != tokens[tokens[j].root_index].root_index
                || tokens[tokens[j + 1].root_index].kind == ARRAY) {
                    int target = tokens[j + 1].root_index;
                    int cur_node = j;
                    for (;;) {

                        if(tokens[tokens[cur_node].root_index].kind == ARRAY) {
                            cursor += cat_raw(output + cursor, compact ? "" : "\n");
                            --ident;
                            cursor += (cs_memcpy(output + cursor, print_ident(ident, compact), cs_strlen(print_ident(ident, compact))), cs_strlen(print_ident(ident, compact)));
                            cursor += cat_raw(output + cursor, "]");
                        }
                        else if(tokens[tokens[cur_node].root_index].kind == OBJECT) {
                            cursor += cat_raw(output + cursor, compact ? "" : "\n");
                            --ident;
                            cursor += (cs_memcpy(output + cursor, print_ident(ident, compact), cs_strlen(print_ident(ident, compact))), cs_strlen(print_ident(ident, compact)));
                            cursor += cat_raw(output + cursor, "}");
                        }
                        if(tokens[(cur_node = tokens[cur_node].root_index)].root_index == target) {
                            break;
                        }
                    }
                }
            }
            if (j + 1 < max && (
                tokens[tokens[j].root_index].kind == STRING || tokens[tokens[j].root_index].kind == ARRAY
            ) && tokens[j].kind != ARRAY && tokens[j].kind != OBJECT) {
                cursor += cat_raw(output + cursor, compact ? (u8*)"," : (u8*)",\n");
            }
        }
    }

    return output;
#undef cat
#undef cat_raw
}

#endif //JSON_JSON_H /* Automatically added header guard. */
