#include "json.h"

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


EXPORT void start_string(struct state * state) {
    state->string_cursor += (*state->string_pool)[state->string_cursor] + 1;
}

EXPORT void push_string(struct state* state, char* string, int length) {
    memcpy(*(state->string_pool) + state->string_cursor + 1 + (*state->string_pool)[state->string_cursor], string, length);
    (*state->string_pool)[state->string_cursor] += length;
}

EXPORT void close_root(struct state * state) {
    state->root_index = (*state->tokens_stack)[state->root_index].root_index;
}

EXPORT void push_root(struct state * state) {
    state->root_index = state->token_cursor - 1;
}

EXPORT void push_token(enum kind kind, void * address, struct state * state) {
    (*state->tokens_stack)[state->token_cursor++] = (struct token) {.kind=kind, .root_index=state->root_index, .address=address};
}

EXPORT int rjson(unsigned char* string, size_t len, struct state* state) {

#define peek_at(where) string[state->ordinal + where]
#define set_state_and_advance_by(which_, advance_) state->kind = which_; state->ordinal += advance_
    if (state->tokens_stack == NULL) {
        state->tokens_stack = &tokens__;
    }
    if (state->string_pool == NULL) {
        state->string_pool = &string_pool__;
    }
    memset(*state->tokens_stack, 0, sizeof *state->tokens_stack);
    push_token(ROOT, NULL, state);
    memset(*state->string_pool, 0, sizeof *state->string_pool);

    // todo: make fully reentrant
    // todo: make ANSI/STDC compatible
    // todo: make libc optional
    // todo: complete unicode support?

    if (state->ordinal == 0) {
        state->error = JSON_ERROR_NO_ERRORS;
        state->kind = WHITESPACE_BEFORE_VALUE;
    }

    for(;;) {
        switch (state->kind) {
            case WHITESPACE_BEFORE_VALUE: {
                set_state_and_advance_by(EXPECT_VALUE, len_whitespace(string, state));
                break;
            }

            case WHITESPACE_AFTER_VALUE: {
                set_state_and_advance_by(AFTER_VALUE, len_whitespace(string, state));
                break;
            }

            case EXPECT_VALUE:
            {
                if (strncmp("true", (char*)string + state->ordinal, strlen("true")) == 0) {
                    push_token(TRUE, NULL, state);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, strlen("true"));
                }
                else if (strncmp("false", (char*)string + state->ordinal, strlen("false")) == 0) {
                    push_token(FALSE, NULL, state);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, strlen("false"));
                }
                else if (strncmp("null", (char *)string + state->ordinal, strlen("null")) == 0) {
                    push_token(JSON_NULL, NULL, state);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, strlen("null"));
                }
                else if (peek_at(0) == '"') {
                    start_string(state);
                    set_state_and_advance_by(FOUND_OPEN_QUOTE, 1);
                }
                else if (peek_at(0) == '[') {
                    push_token(ARRAY, NULL, state);
                    push_root(state);
                    set_state_and_advance_by(OPEN_ARRAY, 1);
                }
                else if (peek_at(0) == '{') {
                    push_token(OBJECT, NULL, state);
                    push_root(state);
                    set_state_and_advance_by(OPEN_ASSOC, 1);
                }
                else if (in(digit_starters, peek_at(0))) {
                    start_string(state);
                    set_state_and_advance_by(START_NUMBER, 0);
                }
                else if (state->tokens_stack[state->root_index]->kind != ROOT) {
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
                if ((*state->tokens_stack)[state->root_index].kind == ROOT) {
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
                else if((*state->tokens_stack)[state->root_index].kind == ARRAY) {
                    set_state_and_advance_by(ARRAY_AFTER_VALUE, 0);
                }
                else if((*state->tokens_stack)[state->root_index].kind == OBJECT) {
                    set_state_and_advance_by(ASSOC_AFTER_VALUE, 0);
                }
                else if((*state->tokens_stack)[state->root_index].kind == STRING) {
                    set_state_and_advance_by(ASSOC_AFTER_INNER_VALUE, 0);
                }

                break;
            }
            case OPEN_ARRAY: {
                if (peek_at(len_whitespace(string, state)) == ']') {
                    close_root(state);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, len_whitespace(string, state) + 1);
                } else {
                    set_state_and_advance_by(WHITESPACE_BEFORE_VALUE, 0);
                }

                break;
            }
            case OPEN_ASSOC: {
                if (peek_at(len_whitespace(string, state)) == '}') {
                    close_root(state);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, len_whitespace(string, state) + 1);
                } else {
                    set_state_and_advance_by(ASSOC_WHITESPACE_BEFORE_KEY, 0);
                }
                break;
            }
            case FOUND_OPEN_QUOTE:
            {
                if (peek_at(0) == '\\') {
                    set_state_and_advance_by(LITERAL_ESCAPE, 1);
                } else if (peek_at(0) == '"') {
                    set_state_and_advance_by(CLOSE_STRING, 1);
                } else {
                    set_state_and_advance_by(IN_STRING, 0);
                }

                break;
            }
            case LITERAL_ESCAPE: {
                if (peek_at(0) == '\\') {
                    push_string(state, "\\\\", 2);
                }
                else if (peek_at(0) == '"') {
                    push_string(state, "\\\"", 2);
                }
                else if (peek_at(0) == '/') {
                    push_string(state, "\\/", 2);
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
                    push_string(state, "\\u", 2);
                    push_string(state, (char*)string+state->ordinal+1, 4);
                    set_state_and_advance_by(IN_STRING, 5);
                    break;
                }
                else {
                    state->error = JSON_ERROR_INVALID_ESCAPE_SEQUENCE;
                }
                set_state_and_advance_by(IN_STRING, 1);

                break;
            }
            case IN_STRING: {
                if (!remaining(len, state))
                {
                    state->error = JSON_ERROR_JSON_TOO_SHORT;
                }
                else if (peek_at(0) == '\\')
                {
                    set_state_and_advance_by(LITERAL_ESCAPE, 1);
                }
                else if (peek_at(0) == '"')
                {
                    set_state_and_advance_by(CLOSE_STRING, 1);
                }
                else if (peek_at(0) == '\b') {
                    push_string(state, "\\b", 2);
                    set_state_and_advance_by(IN_STRING, 1);
                }
                else if (peek_at(0) == '\f') {
                    push_string(state, "\\f", 2);
                    set_state_and_advance_by(IN_STRING, 1);
                }
                else if (peek_at(0) == '\n') {
                    push_string(state, "\\n", 2);
                    set_state_and_advance_by(IN_STRING, 1);
                }
                else if (peek_at(0) == '\r') {
                    push_string(state, "\\r", 2);
                    set_state_and_advance_by(IN_STRING, 1);
                }
                else if (peek_at(0) == '\t') {
                    push_string(state, "\\t", 2);
                    set_state_and_advance_by(IN_STRING, 1);
                }
                else if(peek_at(0) < ' ')
                {
                    push_string(state, "\\u00", 4);
                    push_string(state, (char[2]){num_to_hex[peek_at(0)/16], num_to_hex[peek_at(0)%16]}, 2);
                    set_state_and_advance_by(IN_STRING, 1);
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
                    push_string(state, "\\u00", 4);
                    unsigned char decoded = (string[state->ordinal]&0x1Fu<<5u)|(string[state->ordinal+1]&0x7Fu);
                    push_string(state, (char[2]){num_to_hex[decoded/16], num_to_hex[decoded%16]}, 2);
                    set_state_and_advance_by(IN_STRING, 2);
                }
                else {
                    push_string(state, (char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(IN_STRING, 1);
                }

                if(state->ordinal >= 2 && ((peek_at(-2) & 0xe0u) == 0xe0)) {
                    state->error = JSON_ERROR_UTF16_NOT_SUPPORTED_YET;
                }

                break;
            }
            case START_NUMBER: {
                if (peek_at(0) == '-') {
                    push_string(state, (char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(NUMBER_AFTER_MINUS, 1);
                } else {
                    set_state_and_advance_by(NUMBER_AFTER_MINUS, 0);
                }

                break;
            }
            case NUMBER_AFTER_MINUS:
            {
                if (peek_at(0) == '0') {
                    push_string(state, (char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(EXPECT_FRACTION, 1);
                } else if (in(digits19, peek_at(0))) {
                    push_string(state, (char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(IN_NUMBER, 1);
                } else {
                    state->error = JSON_ERROR_INVALID_NUMBER;
                }

                break;
            }
            case IN_NUMBER: {
                if (in(digits, peek_at(0))) {
                    push_string(state, (char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(IN_NUMBER, 1);
                } else {
                    set_state_and_advance_by(EXPECT_FRACTION, 0);
                }

                break;
            }
            case EXPECT_FRACTION: {
                if (peek_at(0) == '.') {
                    push_string(state, (char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(IN_FRACTION, 1);
                } else {
                    set_state_and_advance_by(EXPECT_EXPONENT, 0);
                }

                break;
            }
            case EXPECT_EXPONENT: {
                if (peek_at(0) == 'e' || peek_at(0) == 'E') {
                    push_string(state, (char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(EXPONENT_EXPECT_PLUS_MINUS, 1);
                } else {
                    push_token(NUMBER, (*state->string_pool) + state->string_cursor, state);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, 0);
                }

                break;
            }
            case IN_FRACTION: {
                if (in(digits, peek_at(0))) {
                    push_string(state, (char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(IN_FRACTION_DIGIT, 1);
                } else {
                    state->error = JSON_ERROR_INVALID_NUMBER;
                }

                break;
            }
            case IN_FRACTION_DIGIT: {
                if (in(digits, peek_at(0))) {
                    push_string(state, (char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(IN_FRACTION_DIGIT, 1);
                } else {
                    set_state_and_advance_by(EXPECT_EXPONENT, 0);
                }

                break;
            }
            case EXPONENT_EXPECT_PLUS_MINUS: {
                if (peek_at(0) == '+' || peek_at(0) == '-') {
                    push_string(state, (char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(EXPECT_EXPONENT_DIGIT, 1);
                } else {
                    set_state_and_advance_by(EXPECT_EXPONENT_DIGIT, 0);
                }

                break;
            }
            case EXPECT_EXPONENT_DIGIT: {
                if (in(digits, peek_at(0))) {
                    push_string(state, (char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(IN_EXPONENT_DIGIT, 1);
                } else {
                    state->error = JSON_ERROR_INVALID_NUMBER;
                }

                break;
            }
            case IN_EXPONENT_DIGIT: {
                if (in(digits, peek_at(0))) {
                    push_string(state, (char[]) {peek_at(0)}, 1);
                    set_state_and_advance_by(IN_EXPONENT_DIGIT, 1);
                } else {
                    push_token(NUMBER, (*state->string_pool) + state->string_cursor, state);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, 0);
                }

                break;
            }
            case ARRAY_AFTER_VALUE: {
                if(peek_at(0) == ',') {
                    set_state_and_advance_by(WHITESPACE_BEFORE_VALUE, 1);
                } else if(peek_at(0) == ']') {
                    close_root(state);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, 1);
                } else {
                    state->error = JSON_ERROR_INVALID_CHARACTER_IN_ARRAY;
                }

                break;
            }
            case ASSOC_AFTER_VALUE: {
                close_root(state);
                set_state_and_advance_by(WHITESPACE_AFTER_VALUE, 0);

                break;
            }
            case ASSOC_WHITESPACE_BEFORE_KEY: {
                set_state_and_advance_by(ASSOC_EXPECT_KEY, len_whitespace(string, state));

                break;
            }
            case ASSOC_EXPECT_KEY: {
                if(peek_at(0) == '"') {
                    start_string(state);
                    set_state_and_advance_by(IN_STRING, 1);
                } else {
                    state->error = JSON_ERROR_ASSOC_EXPECT_STRING_A_KEY;
                }
                break;
            }
            case ASSOC_AFTER_KEY_WHITESPACE: {
                set_state_and_advance_by(ASSOC_EXPECT_COLON, len_whitespace(string, state));

                break;
            }
            case CLOSE_STRING: {
                push_token(STRING, *(state->string_pool) + state->string_cursor, state);
                if ((*state->tokens_stack)[state->root_index].kind == OBJECT) {
                    push_root(state);
                    set_state_and_advance_by(ASSOC_AFTER_KEY_WHITESPACE, 0);
                } else {
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, 0);
                }

                break;
            }
            case ASSOC_EXPECT_COLON: {
                if(peek_at(0) == ':') {
                    set_state_and_advance_by(WHITESPACE_BEFORE_VALUE, 1);
                } else {
                    state->error = JSON_ERROR_ASSOC_EXPECT_COLON;
                }
                break;
            }
            case ASSOC_AFTER_INNER_VALUE: {
                if(peek_at(0) == ',') {
                    close_root(state);
                    set_state_and_advance_by(ASSOC_WHITESPACE_BEFORE_KEY, 1);
                } else if (peek_at(0) == '}'){
                    close_root(state);
                    set_state_and_advance_by(ASSOC_AFTER_VALUE, 1);
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
#undef set_state_and_advance_by
}

EXPORT void print_debug(struct state * state) {
    int j;
    for (j = 0; j < state->token_cursor; ++j) {
        printf("%d: kind: %s, root: %d", j, (char*[]){
                "UNSET", "ROOT", "TRUE", "FALSE", "JSON_NULL",
                "STRING", "NUMBER", "ARRAY", "OBJECT", "OBJECT_KEY"
        }[(*state->tokens_stack)[j].kind],
                (*state->tokens_stack)[j].root_index);
        if((*state->tokens_stack)[j].kind == STRING || (*state->tokens_stack)[j].kind == NUMBER) {
            char dest[STRING_POOL_SIZE] = {0};
            printf(", value: %s",
                   (char*)memcpy(dest, (char*)((*state->tokens_stack)[j].address)+1, *((char*)(*state->tokens_stack)[j].address))
                   );
        }
        puts("");
    }
}

static char ident_s[0x80];
static char * print_ident(int ident) {
    memset(ident_s, ' ', ident * 2);
    ident_s[ident * 2] = '\0';

    return ident_s;
}


EXPORT char * to_string(struct token tokens_[0x200], int max) {
    // todo: make the caller handle the buffer

    struct token *tokens = tokens_;

    static char output[0x1600];
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
