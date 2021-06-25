#include "json.h"

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
    /* todo: memmove if we insert */
    /* todo: support strings longer than 256 characters */
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
    tokens->stack[(tokens->max)++] = (struct token) {
        .kind=kind, .root_index=root_index, .address=address
    };
}

EXPORT void
insert_token(struct cisson_state * state, char *token, struct token* root) {
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
        START_AND_INSERT_TOKEN(state, kind, token, root - state->tokens.stack);
    }
    if(in("{[:#", token[0])) PUSH_ROOT(state);
}

EXPORT void
stream_tokens_(struct cisson_state * state, struct token * where, char separator, char *stream, size_t length) {
    size_t i = 0;
    int old_root = state->root_index;
    state->root_index = (int)(where - state->tokens.stack);
    while (i < length) {
        size_t token_length = 0;
        while (i + token_length < length && stream[i + token_length] != separator) {
            token_length++;
        }
        stream[i + token_length] = '\0';
        push_token(state, &stream[i]);
        i = i + token_length + sizeof separator;
    }
    state->root_index = old_root;
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
    state->tokens.stack = stack;
    state->strings.pool = pool;
    state->root_index = -1;
}

static struct token *
next_child(struct tokens *  tokens, struct token * root, struct token * current) {
    int root_idx = (int)(root - tokens->stack);

    int start = (root_idx + 1) % tokens->max;

    if (current == root) {
        current = NULL;
    }

    if (current != NULL) {
        while (current->root_index != root_idx) {
            current = &tokens->stack[current->root_index];
        }
        int current_idx = (int)(current - tokens->stack);
        start = (current_idx + 1) % tokens->max;
    }
    int i;
    for(
            i = start;
            i != root_idx;
            i++, i%=tokens->max) {
        if (tokens->stack[i].root_index == root_idx) {
            return &tokens->stack[i];
        }
    }
    return NULL;
}

EXPORT struct token *
query_(struct cisson_state * state, size_t length, char query[va_(length)]) {
    /* todo : unescape ~0, ~1, ~2 */
    size_t i = 0;
    struct token * cursor = &state->tokens.stack[1];

    while (i < length) {
        if (query[i] == '/') {
            if(cursor->kind != OBJECT && cursor->kind != ARRAY) {
                return NULL;
            }
            i++;
            size_t token_length = 0;
            while (i + token_length < length && query[i + token_length] != '/') {
                token_length++;
            }
            unsigned char buffer[0x80] = { '"' * (cursor->kind == OBJECT) };
            cs_memcpy(&buffer[1 * (cursor->kind == OBJECT)], &query[i], token_length);
            buffer[1 + token_length] = '"' * (cursor->kind == OBJECT);

            if (cursor->kind == ARRAY) {
                int index = 0;
                int j;
                for (j = 0; buffer[j]; j++) {
                    index *= 10;
                    index += buffer[j] - '0';
                }
                struct token * cur = NULL;
                do {
                    cur = next_child(&state->tokens, cursor, cur);
                } while (index--);
                cursor = cur;
            } else if (cursor->kind == OBJECT) {
                int index = state->tokens.max;
                struct token * cur = NULL;
                do {
                    cur = next_child(&state->tokens, cursor, cur);
                    if (cs_memcmp(cur->address + 1,
                                  buffer,
                                  token_length + 2) == 0) {
                        if((i + token_length + 1 < length &&
                            query[i+token_length] == '/' && query[i+token_length+1] == '<')) {
                            return cur;
                        }
                            cursor = next_child(
                                    &state->tokens,
                                    cur,
                                    NULL);

                        break;
                    }
                } while (index--);
            }
            i = i + token_length - 1;
        }
        i++;
    }

    return cursor;
}

EXPORT int
aindex(struct token* stack, struct token* which) {
    return (int)(which - stack);
}

EXPORT void
delete(struct token* which) {
    which->root_index = -2;
}

EXPORT void
move(struct cisson_state* state, struct token* which, struct token* where) {
    which->root_index = aindex(state->tokens.stack, where);
}

EXPORT void
srename_(struct cisson_state* state, struct token* which, int len, char* new_name) {
    START_STRING(state);
    PUSH_STRING(state, "\"", 1);
    PUSH_STRING(state, new_name, len);
    PUSH_STRING(state, "\"", 1);
    which->address = state->strings.pool + state->strings.cursor;
}


EXPORT enum json_errors
inject_(size_t len,
       unsigned char text[va_(len)],
       struct cisson_state * state,
       struct token * where) {
    enum states old_state = state->cur_state;
    int old_root = state->root_index;
    state->root_index = (int)(where - state->tokens.stack);
    state->cur_state = EXPECT_BOM;
    enum json_errors error = rjson_(
            cs_strlen((char *) text),
            (unsigned char *) text, state);
    state->cur_state = old_state;
    state->root_index = old_root;
    return error;
}

EXPORT enum json_errors
rjson_(size_t len,
       unsigned char *cursor,
       struct cisson_state * state) {

#define peek_at(where) cursor[where]
#define SET_STATE_AND_ADVANCE_BY(which_, advance_) \
  state->cur_state = which_; cursor += advance_

    if (state->tokens.stack == NULL) {
        start_state(
                state,
                static_stack,
                sizeof static_stack,
                static_pool,
                sizeof static_pool);
    }

    if(state->tokens.stack->kind == UNSET) {
        START_AND_PUSH_TOKEN(state, ROOT, "#root");
        PUSH_ROOT(state);
    }

    enum json_errors error = JSON_ERROR_NO_ERRORS;
    unsigned char * final = cursor + len;

    // todo: fully test reentry
    // todo: make ANSI/STDC compatible
    // todo: complete unicode support?
    // todo: add jasmine mode? aka not copy strings+numbers ?
    // todo: pedantic mode? (forbid duplicate keys, enforce order, cap integer values to INT_MAX...)
    // todo: test for overflows
    // fixme: check for bounds
    // todo: example with slowly filled array
    // todo: no memory move mode (simple tokenizer)
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
                else if (state->tokens.stack[state->root_index].kind != ROOT) {
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
                if (state->tokens.stack[state->root_index].kind == ROOT) {
                    if (remaining(final, cursor + len_whitespace(cursor))) {
                        error = JSON_ERROR_NO_SIBLINGS;
#ifdef WANT_JSON1
                    } else if(state->mode == JSON1 && tokens[state->max-1].kind != OBJECT) {
                        state->error = JSON_ERROR_JSON1_ONLY_ASSOC_ROOT;
#endif
                    } else {
                        goto exit;
                    }
                }
                else if(state->tokens.stack[state->root_index].kind == ARRAY) {
                    SET_STATE_AND_ADVANCE_BY(ARRAY_AFTER_VALUE, 0);
                }
                else if(state->tokens.stack[state->root_index].kind == STRING) {
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
                if ((state->tokens.stack)[state->root_index].kind == OBJECT) {
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
    struct token *stack = tokens->stack;

    memset(output, 0, sizeof(output));
    for (j = 0; j < tokens->max; ++j) {
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
static char * print_ident(int depth, int indent) {
    if (!depth) return "";
    cs_memset(ident_s, ' ', indent * depth);
    ident_s[indent * depth] = '\0';
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
to_string_(struct tokens * res tokens, struct token * start, int indent) {
    // todo: make the caller handle the buffer

#define cat(where, string, token) (\
    minified_string((where), (string), *((char*)(token)->address)) \
)
#define cat_raw(where, string) ( \
    cs_memcpy((where), (string), cs_strlen((string))), cs_strlen((string)) \
)

    static unsigned char output[STRING_POOL_SIZE];
    cs_memset(output, 0, sizeof output);
    size_t cursor = 0;

    struct token *stack = tokens->stack;
    if (start == NULL) {
        start = &stack[0];
    }

    int depth = 0;
    struct token * tok = start;
    struct token * active_root = start;
    unsigned char buf[STRING_POOL_SIZE] = {0};
    do {
        if (stack[tok->root_index].kind != STRING) {
            cursor += cat_raw(output + cursor, print_ident(depth, indent));
        }

        if(tok->kind != ROOT) {
            cs_memcpy(buf, (char *) tok->address + 1,
                      *((char *) tok->address));
            cursor += cat(output + cursor, buf, tok);
            cs_memset(buf, 0, *((char *) tok->address));
        }

        if (stack[tok->root_index].kind == OBJECT && tok->kind == STRING) {
            cursor += cat_raw(output + cursor, indent ? ": " :  ":");
        }

        if(tok->kind == ARRAY || tok->kind == OBJECT) {
            cursor += cat_raw(output + cursor, !indent ? "" : "\n");
            depth++;
        }

        while(next_child(tokens, active_root, tok) == NULL) {
            if((stack[tok->root_index].kind == ARRAY || stack[tok->root_index].kind == OBJECT) && active_root->kind != STRING) {
                cursor += cat_raw(output + cursor, !indent ? "" : "\n");
                --depth, depth < 0 ? depth = 0 : 0;
                cursor += cat_raw(output + cursor,
                                  print_ident(depth,
                                              indent));
            }

            char end[2] = {"\0]}"[in((char[]){ARRAY, OBJECT}, active_root->kind)], '\0'};
            cursor += cat_raw(output + cursor, end);

            if (active_root == start && next_child(tokens, active_root, tok) == NULL) {
                goto end;
            }
            active_root = &stack[active_root->root_index];
        }

        if ((
                stack[tok->root_index].kind == STRING || stack[tok->root_index].kind == ARRAY
        ) && tok->kind != ARRAY && tok->kind != OBJECT) {
            cursor += cat_raw(output + cursor,
                              indent ? ",\n" : ",");
        }

        tok = next_child(tokens, active_root, tok);
        if (tok->kind == OBJECT || tok->kind == ARRAY || tok->kind == STRING && stack[tok->root_index].kind == OBJECT) {
            active_root = tok;
        }
    } while(1);

    end:
    return output;
#undef cat
#undef cat_raw

}