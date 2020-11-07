/* Automatically generated to be used as a single header replacement
 * of the full library, do not edit. */
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
/* __/ */
#define START_STRING(state_) start_string(&(state_)->string_cursor, (state_)->string_pool)
#define PUSH_STRING(state_, string_, length_) push_string(&(state_)->string_cursor, (state_)->string_pool, (string_), (length_))
#define CLOSE_ROOT(state_) close_root((*state_).tokens_stack, &(*state_).root_index)
#define PUSH_ROOT(state_) push_root(&(state_)->root_index, &(state_)->token_cursor)
#define PUSH_TOKEN(kind_, address_, state_) push_token((kind_), (address_), (state_)->tokens_stack, &(state_)->token_cursor, (state_)->root_index)
#define PUSH_STRING_TOKEN(kind_, state_) PUSH_TOKEN((kind_), (state_)->string_pool + (state_)->string_cursor, (state_))

#endif //JSON_JSON_H


int static inline len_whitespace(unsigned char* string, struct state * state) {
    int count = 0;
    while(string[state->ordinal + count] != '\0' && strchr(whitespaces, string[state->ordinal + count]) != NULL) {
        count++;
    }

    return count;
}

static inline size_t remaining(int max, struct state* state) {
    return max - state->ordinal;
}

static inline int in(char* hay, char needle) {
    return needle != '\0' && strchr(hay, needle);
}

EXPORT void start_string(unsigned char * cursor, const unsigned char pool[STRING_POOL_SIZE]) {
    *cursor += pool[*cursor] + 1;
}

EXPORT void push_string(const unsigned char * cursor, unsigned char pool[STRING_POOL_SIZE], char* string, int length) {
    memcpy(pool + *cursor + 1 + pool[*cursor], string, length);
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
    PUSH_TOKEN(ROOT, NULL, state);

    // fixme: complete example with self managed memory.
    // todo: fully test reentrance
    // todo: make ANSI/STDC compatible
    // todo: make libc optional
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
                SET_STATE_AND_ADVANCE_BY(EXPECT_VALUE, len_whitespace(string, state));
                break;
            }

            case WHITESPACE_AFTER_VALUE: {
                SET_STATE_AND_ADVANCE_BY(AFTER_VALUE, len_whitespace(string, state));
                break;
            }

            case EXPECT_VALUE:
            {
                if (strncmp("true", (char*)string + state->ordinal, strlen("true")) == 0) {
//                    PUSH_TOKEN(TRUE, NULL, state);
                    push_token((TRUE), (((void *) 0)), (state)->tokens_stack, &(state)->token_cursor, (state)->root_index);
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_AFTER_VALUE, strlen("true"));
                }
                else if (strncmp("false", (char*)string + state->ordinal, strlen("false")) == 0) {
                    PUSH_TOKEN(FALSE, NULL, state);
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_AFTER_VALUE, strlen("false"));
                }
                else if (strncmp("null", (char *)string + state->ordinal, strlen("null")) == 0) {
                    PUSH_TOKEN(JSON_NULL, NULL, state);
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_AFTER_VALUE, strlen("null"));
                }
                else if (peek_at(0) == '"') {
                    START_STRING(state);
                    SET_STATE_AND_ADVANCE_BY(FOUND_OPEN_QUOTE, 1);
                }
                else if (peek_at(0) == '[') {
                    PUSH_TOKEN(ARRAY, NULL, state);
                    PUSH_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(OPEN_ARRAY, 1);
                }
                else if (peek_at(0) == '{') {
                    PUSH_TOKEN(OBJECT, NULL, state);
                    PUSH_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(OPEN_ASSOC, 1);
                }
                else if (in(digit_starters, peek_at(0))) {
                    START_STRING(state);
                    SET_STATE_AND_ADVANCE_BY(START_NUMBER, 0);
                }
                else if (tokens[state->root_index]->kind != ROOT) {
                    state->error = JSON_ERROR_ASSOC_EXPECT_VALUE;
                }
                else if (remaining(len, state)) {
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
                    if (remaining(len, state)) {
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
                if (peek_at(len_whitespace(string, state)) == ']') {
                    CLOSE_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_AFTER_VALUE, len_whitespace(string, state) + 1);
                } else {
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_BEFORE_VALUE, 0);
                }

                break;
            }
            case OPEN_ASSOC: {
                if (peek_at(len_whitespace(string, state)) == '}') {
                    CLOSE_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_AFTER_VALUE, len_whitespace(string, state) + 1);
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
                if (!remaining(len, state))
                {
                    state->error = JSON_ERROR_JSON_TOO_SHORT;
                }
                else if (peek_at(0) == '\\')
                {
                    SET_STATE_AND_ADVANCE_BY(LITERAL_ESCAPE, 1);
                }
                else if (peek_at(0) == '"')
                {
                    SET_STATE_AND_ADVANCE_BY(CLOSE_STRING, 1);
                }
                else if (peek_at(0) == '\b') {
                    PUSH_STRING(state, "\\b", 2);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 1);
                }
                else if (peek_at(0) == '\f') {
                    PUSH_STRING(state, "\\f", 2);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 1);
                }
                else if (peek_at(0) == '\n') {
                    PUSH_STRING(state, "\\n", 2);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 1);
                }
                else if (peek_at(0) == '\r') {
                    PUSH_STRING(state, "\\r", 2);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 1);
                }
                else if (peek_at(0) == '\t') {
                    PUSH_STRING(state, "\\t", 2);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 1);
                }
                else if(peek_at(0) < ' ')
                {
                    PUSH_STRING(state, "\\u00", 4);
                    PUSH_STRING(state, ((char[2]){hexdigits[peek_at(0)/16], hexdigits[peek_at(0)%16]}), 2);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 1);
                }
//                else if(peek_at(0) > 0xffff) { /* In UTF16 realm */
//                    push_string(state, "\\u00", 4);
//                    push_string(state, (char[]){num_to_hex[peek_at(0)/16], num_to_hex[peek_at(0)%16]}, 2);
//                    set_state_and_advance_by(IN_STRING, 1);
//                }
                else if(peek_at(0) > 0xFF / 2) { /* In extended ASCII */
                    if((peek_at(0) & 0xe0u) == 0xe0) { /* 3 highest bits are set */
                        state->error = JSON_ERROR_UTF16_NOT_SUPPORTED_YET;
                        break;
                    }
                    PUSH_STRING(state, "\\u00", 4);
                    unsigned char decoded = (string[state->ordinal]&0x1Fu<<5u)|(string[state->ordinal+1]&0x7Fu);
                    PUSH_STRING(state, ((char[2]){hexdigits[decoded/16], hexdigits[decoded%16]}), 2);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 2);
                }
                else {
                    PUSH_STRING(state, (char[]){peek_at(0)}, 1);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 1);
                }

                if(state->ordinal >= 2 && ((peek_at(-2) & 0xe0u) == 0xe0)) {
                    state->error = JSON_ERROR_UTF16_NOT_SUPPORTED_YET;
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
                SET_STATE_AND_ADVANCE_BY(ASSOC_EXPECT_KEY, len_whitespace(string, state));

                break;
            }
            case ASSOC_EXPECT_KEY: {
                if(peek_at(0) == '"') {
                    START_STRING(state);
                    SET_STATE_AND_ADVANCE_BY(IN_STRING, 1);
                } else {
                    state->error = JSON_ERROR_ASSOC_EXPECT_STRING_A_KEY;
                }
                break;
            }
            case ASSOC_AFTER_KEY_WHITESPACE: {
                SET_STATE_AND_ADVANCE_BY(ASSOC_EXPECT_COLON, len_whitespace(string, state));

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

EXPORT char* print_debug(struct state * state) {
    int j;
    int cursor = 0;
    struct token *tokens = state->tokens_stack;

    static char output[STRING_POOL_SIZE];
    for (j = 0; j < state->token_cursor; ++j) {
        cursor += sprintf(output, "%d: kind: %s, root: %d", j, (char*[]){
                "UNSET", "ROOT", "TRUE", "FALSE", "JSON_NULL",
                "STRING", "NUMBER", "ARRAY", "OBJECT", "OBJECT_KEY"
        }[tokens[j].kind],
                tokens[j].root_index);
        if(tokens[j].kind == STRING || tokens[j].kind == NUMBER) {
            char dest[STRING_POOL_SIZE] = {0};
            cursor += sprintf(output + cursor, ", value: %s",
                   (char*)memcpy(dest, (char*)(tokens[j].address)+1, *((char*)tokens[j].address))
                   );
        }
        snprintf(output + cursor, 1, "");

    }

    return output;
}

static char ident_s[0x80];
static char * print_ident(int ident) {
    memset(ident_s, ' ', ident * 2);
    ident_s[ident * 2] = '\0';

    return ident_s;
}


EXPORT char * to_string(struct token tokens_[0x200], int max) {
    // todo: make the caller handle the buffer
    // todo: add compact output for tests

    struct token *tokens = tokens_;

    static char output[STRING_POOL_SIZE];
    memset(output, 0, sizeof output);
    int cursor = 0;
    int ident = 0;
    int j;
    for (j = 1; j < max; ++j) {
        if (tokens[tokens[j].root_index].kind == STRING) {
            cursor += sprintf(output + cursor, ": ");
        } else {
            cursor += sprintf(output + cursor, "%s", print_ident(ident));
        }

        if (tokens[j].kind == TRUE) cursor += sprintf(output + cursor, "true");
        if (tokens[j].kind == FALSE) cursor += sprintf(output + cursor, "false");
        if (tokens[j].kind == JSON_NULL) cursor += sprintf(output + cursor, "null");
        if (tokens[j].kind == STRING || tokens[j].kind == NUMBER) {
            char dest[STRING_POOL_SIZE] = {0};
            cursor += sprintf(
                    output + cursor, (char *[]) {"\"%s\"", "%s"}[tokens[j].kind == NUMBER],
                    memcpy(dest, (char*)tokens[j].address+1, *((char *) tokens[j].address))
            );
        }
        if (tokens[j].kind == ARRAY) cursor += sprintf(output + cursor, "[\n"), ident++;
        if (tokens[j].kind == OBJECT) cursor += sprintf(output + cursor, "{\n"), ident++;
        if (j <= max) {
            if (tokens[j + 1].root_index < tokens[j].root_index) {
                if (j + 1 == max
                || tokens[j + 1].root_index != tokens[tokens[j].root_index].root_index
                || tokens[tokens[j + 1].root_index].kind == ARRAY) {
                    int target = tokens[j + 1].root_index;
                    int cur_node = j;
                    for (;;) {

                        if(tokens[tokens[cur_node].root_index].kind == ARRAY) {
                            cursor += sprintf(output + cursor, "\n%s]", print_ident(--ident));
                        }
                        else if(tokens[tokens[cur_node].root_index].kind == OBJECT) {
                            cursor += sprintf(output + cursor, "\n%s}", print_ident(--ident));
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
                cursor += sprintf(output + cursor, ",\n");
            }
        }
    }
    snprintf(output + cursor, 1, "");
    return output;
}
