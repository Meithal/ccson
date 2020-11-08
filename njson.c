#include "json.h"

static inline int in(char* hay, unsigned char needle) {
    if (needle == '\0') return 0;
    for (;*hay; hay++) {
        if(*hay == needle) return 1;
    }
    return 0;
}

static inline size_t
len_whitespace(unsigned char* string) {
    int count = 0;
    while(in(whitespaces, string[count])) {
        count++;
    }
    return count;
}

static inline size_t
remaining(unsigned char *max, unsigned char *where) {
    return max - where;
}

EXPORT void
start_string(unsigned int * cursor,
             const unsigned char pool[STRING_POOL_SIZE]) {
    *cursor += pool[*cursor] + 1;
}

EXPORT void
push_string(const unsigned int *cursor,
            unsigned char *pool,
            char* string,
            int length) {
    cs_memcpy(pool + *cursor + 1 + pool[*cursor], string, length);
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
push_token(  // fixme
        enum kind kind,
        void * address,
        struct token *tokens,
        int * token_cursor,
        int root_index) {
    tokens[(*token_cursor)++] = (struct token) {
        .kind=kind, .root_index=root_index, .address=address
    };
}

EXPORT int
rjson(size_t len, struct state *state) {

#define peek_at(where) state->cursor[where]
#define SET_STATE_AND_ADVANCE_BY(which_, advance_) \
    state->cur_state = which_; state->cursor += advance_

    struct token (*tokens)[MAX_TOKENS] = &state->tokens_stack;

    tokens[0]->kind = UNSET;
    unsigned char * final = state->cursor + len;
    START_AND_PUSH_TOKEN(state, ROOT, "#root");

    // fixme: complete example with self managed memory.
    // todo: fully test reentrance
    // todo: make ANSI/STDC compatible
    // todo: complete unicode support?
    // todo: add jasmine mode? aka not copy strings+numbers ?
    // todo: pedantic mode?

    if (state->cursor == 0) {
        state->error = JSON_ERROR_NO_ERRORS;
        state->cur_state = WHITESPACE_BEFORE_VALUE;
    }

    for(;;) {
        switch (state->cur_state) {
            case WHITESPACE_BEFORE_VALUE: {
                SET_STATE_AND_ADVANCE_BY(EXPECT_VALUE, len_whitespace(state->cursor));
                break;
            }

            case WHITESPACE_AFTER_VALUE: {
                SET_STATE_AND_ADVANCE_BY(AFTER_VALUE, len_whitespace(state->cursor));
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
                else if (remaining(final, state->cursor)) {
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
                    if (remaining(final, state->cursor)) {
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
                if (peek_at(len_whitespace(state->cursor)) == ']') {
                    CLOSE_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_AFTER_VALUE, len_whitespace(state->cursor) + 1);
                } else {
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_BEFORE_VALUE, 0);
                }

                break;
            }
            case OPEN_ASSOC: {
                if (peek_at(len_whitespace(state->cursor)) == '}') {
                    CLOSE_ROOT(state);
                    SET_STATE_AND_ADVANCE_BY(WHITESPACE_AFTER_VALUE, len_whitespace(state->cursor) + 1);
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
                    PUSH_STRING(state, (char*)state->cursor + 1, 4);
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
                SET_STATE_AND_ADVANCE_BY(ASSOC_EXPECT_KEY, len_whitespace(state->cursor));

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
                SET_STATE_AND_ADVANCE_BY(ASSOC_EXPECT_COLON, len_whitespace(state->cursor));

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
static char output[STRING_POOL_SIZE];
EXPORT char* print_debug(struct state * state) {
    int j;
    int cursor = 0;
    struct token *tokens = state->tokens_stack;

    memset(output, 0, sizeof(output));
    for (j = 0; j < state->token_cursor; ++j) {
        char dest[STRING_POOL_SIZE] = {0};

        cursor += snprintf(
                output + cursor, 80,
                "%d: kind: %s, root: %d, value: %s\\n\"\n", j, (char*[]){
                            "UNSET", "ROOT", "TRUE", "FALSE", "JSON_NULL",
                            "STRING", "NUMBER", "ARRAY", "OBJECT"
                    }[tokens[j].kind],
                tokens[j].root_index,
                (char*)memcpy(dest, (char*)(tokens[j].address)+1, *((char*)tokens[j].address))
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

EXPORT unsigned char *to_string_(struct token tokens[MAX_TOKENS], int max, int compact) {
    // todo: make the caller handle the buffer

#define cat(where, string, token) (\
    shortest_safe_string((where), (string), *((char*)(token)->address)\
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
                cursor += cat_raw(output + cursor, compact ? "," : ",\n");
            }
        }
    }

    return output;
#undef cat
#undef cat_raw
}
