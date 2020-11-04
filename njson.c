//
// Created on 15/01/2020.
//

#include "json.h"
#include "assert.h"

size_t len_whitespace(char* string, struct state * state) {
    int count = 0;
    while(string[state->ordinal + count] != '\0' && strchr(whitespaces, string[state->ordinal + count]) != NULL) {
        count++;
    }
    return count;
}

void start_string() {
    string_cursor += string_pool[string_cursor-1] + 1;
}

void push_string(char* string, size_t length) {
    assert(length > 0);
    memcpy(string_pool + string_cursor + string_pool[string_cursor-1], string, length);
    string_pool[string_cursor-1] += length;
}

void close_root(int * root_index) {
    *root_index = tokens[*root_index].root_index;
}

void push_root(int * root_index) {
    *root_index = token_cursor - 1;
}

void push_token(enum kind kind, void * address, int root_index) {
    tokens[token_cursor] = (struct token) {.kind=kind, .root_index=root_index, .address=address};
    token_cursor++;
}

static inline size_t remaining(char * string, struct state* state) {
    return strlen(string + state->ordinal);
}

static inline _Bool in(char* hay, char needle) {
    return needle != '\0' && strchr(hay, needle);
}

#define peek_at(where) string[state->ordinal + where]
#define set_state_and_advance_by(which_, advance_) state->kind = which_; state->ordinal += advance_; to_inc = 0
int rjson(char* string, struct state* state) {

    size_t total_length = strlen(string);

    memcpy(tokens, (struct token [0x200]){(struct token) {.kind=ROOT}}, sizeof (struct token[0x200]));
    token_cursor = 1;
    memset(string_pool, 0, sizeof string_pool);
    string_cursor = 0;
    int root_index = 0;

    // todo: make fully reentrant
    // todo: make ANSI/STDC compatible

    if (state->ordinal == 0) {
        state->error = JSON_ERROR_NO_ERRORS;
        state->kind = WHITESPACE_BEFORE_VALUE;
    }

    while(true) {

        int to_inc = -999;
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
                if (strncmp("true", string + state->ordinal, strlen("true")) == 0) {
                    push_token(TRUE, &JSON_TRUE_SINGLETON, root_index);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, strlen("true"));
                }
                else if (strncmp("false", string + state->ordinal, strlen("false")) == 0) {
                    push_token(FALSE, &JSON_FALSE_SINGLETON, root_index);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, strlen("false"));
                }
                else if (strncmp("null", string + state->ordinal, strlen("null")) == 0) {
                    push_token(JNULL, &JSON_NULL_SINGLETON, root_index);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, strlen("null"));
                }
                else if (peek_at(0) == '"') {
                    start_string();
                    set_state_and_advance_by(FOUND_OPEN_QUOTE, 1);
                }
                else if (peek_at(0) == '[') {
                    push_token(ARRAY, &tokens[root_index], root_index);
                    push_root(&root_index);
                    set_state_and_advance_by(OPEN_ARRAY, 1);
                }
                else if (peek_at(0) == '{') {
                    push_token(OBJECT, &tokens[root_index], root_index);
                    push_root(&root_index);
                    set_state_and_advance_by(OPEN_ASSOC, 1);
                }
                else if (in(digit_starters, peek_at(0))) {
                    start_string();
                    set_state_and_advance_by(START_NUMBER, 0);
                }
                else if (tokens[root_index].kind != ROOT) {
                    state->error = JSON_ERROR_ASSOC_EXPECT_VALUE;
                }
                else if (remaining(string+len_whitespace(string, state), state)) {
                    state->error = JSON_ERROR_INVALID_CHARACTER;
                }
                else {
                    state->error = JSON_ERROR_EMPTY;
                }

                break;
            }
            case AFTER_VALUE:
            {
                if (tokens[root_index].kind == ROOT) {
                    if (remaining(string, state)) {
                        state->error = JSON_ERROR_NO_SIBLINGS;
                    } else {
                        return total_length;
                    }
                }
                else if(tokens[root_index].kind == ARRAY) {
                    set_state_and_advance_by(ARRAY_AFTER_VALUE, 0);
                }
                else if(tokens[root_index].kind == OBJECT) {
                    set_state_and_advance_by(ASSOC_AFTER_VALUE, 0);
                }
                else if(tokens[root_index].kind == STRING) {
                    set_state_and_advance_by(ASSOC_AFTER_INNER_VALUE, 0);
                }
                else {
                    assert(0);
                }

                break;
            }
            case OPEN_ARRAY: {
                if (peek_at(len_whitespace(string, state)) == ']') {
                    close_root(&root_index);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, len_whitespace(string, state) + 1);
                } else {
                    set_state_and_advance_by(WHITESPACE_BEFORE_VALUE, 0);
                }

                break;
            }
            case OPEN_ASSOC: {
                if (peek_at(len_whitespace(string, state)) == '}') {
                    close_root(&root_index);
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
                    push_string("\\", 1);
                }
                else if (peek_at(0) == '"') {
                    push_string("\"", 1);
                }
                else if (peek_at(0) == '/') {
                    push_string("/", 1);
                }
                else if (peek_at(0) == 'b') {
                    push_string("\b", 1);
                }
                else if (peek_at(0) == 'f') {
                    push_string("\f", 1);
                }
                else if (peek_at(0) == 'n') {
                    push_string("\n", 1);
                }
                else if (peek_at(0) == 'r') {
                    push_string("\r", 1);
                }
                else if (peek_at(0) == 't') {
                    push_string("\t", 1);
                } else {
                    state->error = JSON_ERROR_INVALID_ESCAPE_SEQUENCE;
                }
                set_state_and_advance_by(IN_STRING, 1);

                break;
            }
            case IN_STRING: {
                if (!remaining(string + len_whitespace(string, state), state)) {
                    state->error = JSON_ERROR_JSON_TOO_SHORT;
                }
                else if (peek_at(0) == '\\') {
                    set_state_and_advance_by(LITERAL_ESCAPE, 1);
                } else if (peek_at(0) == '"') {
                    set_state_and_advance_by(CLOSE_STRING, 1);
                } else {
                    push_string((char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(IN_STRING, 1);
                }

                break;
            }

            case START_NUMBER: {
                if (peek_at(0) == '-') {
                    push_string((char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(NUMBER_AFTER_MINUS, 1);
                } else {
                    set_state_and_advance_by(NUMBER_AFTER_MINUS, 0);
                }

                break;
            }
            case NUMBER_AFTER_MINUS:
            {
                if (peek_at(0) == '0') {
                    push_string((char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(EXPECT_FRACTION, 1);
                } else if (in(digits19, peek_at(0))) {
                    push_string((char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(IN_NUMBER, 1);
                } else {
                    state->error = JSON_ERROR_INVALID_NUMBER;
                }

                break;
            }
            case IN_NUMBER: {
                if (in(digits, peek_at(0))) {
                    push_string((char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(IN_NUMBER, 1);
                } else {
                    set_state_and_advance_by(EXPECT_FRACTION, 0);
                }

                break;
            }
            case EXPECT_FRACTION: {
                if (peek_at(0) == '.') {
                    push_string((char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(IN_FRACTION, 1);
                } else {
                    set_state_and_advance_by(EXPECT_EXPONENT, 0);
                }

                break;
            }
            case EXPECT_EXPONENT: {
                if (peek_at(0) == 'e' || peek_at(0) == 'E') {
                    push_string((char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(EXPONENT_EXPECT_PLUS_MINUS, 1);
                } else {
                    push_token(NUMBER, string_pool + string_cursor, root_index);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, 0);
                }

                break;
            }
            case IN_FRACTION: {
                if (in(digits, peek_at(0))) {
                    push_string((char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(IN_FRACTION_DIGIT, 1);
                } else {
                    state->error = JSON_ERROR_INVALID_NUMBER;
                }

                break;
            }
            case IN_FRACTION_DIGIT: {
                if (in(digits, peek_at(0))) {
                    push_string((char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(IN_FRACTION_DIGIT, 1);
                } else {
                    set_state_and_advance_by(EXPECT_EXPONENT, 0);
                }

                break;
            }
            case EXPONENT_EXPECT_PLUS_MINUS: {
                if (peek_at(0) == '+' || peek_at(0) == '-') {
                    push_string((char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(EXPECT_EXPONENT_DIGIT, 1);
                } else {
                    set_state_and_advance_by(EXPECT_EXPONENT_DIGIT, 0);
                }

                break;
            }
            case EXPECT_EXPONENT_DIGIT: {
                if (in(digits, peek_at(0))) {
                    push_string((char[]){peek_at(0)}, 1);
                    set_state_and_advance_by(IN_EXPONENT_DIGIT, 1);
                } else {
                    state->error = JSON_ERROR_INVALID_NUMBER;
                }

                break;
            }
            case IN_EXPONENT_DIGIT: {
                if (in(digits, peek_at(0))) {
                    push_string((char[]) {peek_at(0)}, 1);
                    set_state_and_advance_by(IN_EXPONENT_DIGIT, 1);
                } else {
                    push_token(NUMBER, string_pool + string_cursor, root_index);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, 0);
                }

                break;
            }
            case ARRAY_AFTER_VALUE: {
                if(peek_at(0) == ',') {
                    set_state_and_advance_by(WHITESPACE_BEFORE_VALUE, 1);
                } else if(peek_at(0) == ']') {
                    close_root(&root_index);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, 1);
                } else {
                    state->error = JSON_ERROR_INVALID_CHARACTER_IN_ARRAY;
                }

                break;
            }
            case ASSOC_AFTER_VALUE: {
                close_root(&root_index);
                set_state_and_advance_by(WHITESPACE_AFTER_VALUE, 0);

                break;
            }
            case ASSOC_WHITESPACE_BEFORE_KEY: {
                set_state_and_advance_by(ASSOC_EXPECT_KEY, len_whitespace(string, state));

                break;
            }
            case ASSOC_EXPECT_KEY: {
                if(peek_at(0) == '"') {
                    start_string();
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
                push_token(STRING, string_pool + string_cursor, root_index);
                if (tokens[root_index].kind == OBJECT) {
                    push_root(&root_index);
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
                    close_root(&root_index);
                    set_state_and_advance_by(ASSOC_WHITESPACE_BEFORE_KEY, 1);
                } else if (peek_at(0) == '}'){
                    close_root(&root_index);
                    set_state_and_advance_by(ASSOC_AFTER_VALUE, 1);
                } else {
                    state->error = JSON_ERROR_ASSOC_EXPECT_VALUE;
                }
                break;
            }
        }

        if(state->error != JSON_ERROR_NO_ERRORS) {
            return token_cursor;
        }

        assert(to_inc != -999);
    }
#undef peek_at
#undef set_state_and_advance_by
}

void print_debug(void) {
    for (int j = 0; j < token_cursor; ++j) {
        printf("%d: kind: %s, root: %d", j, (char*[]){
                "UNSET", "ROOT", "TRUE", "FALSE", "JNULL",
                "STRING", "NUMBER", "ARRAY", "OBJECT", "OBJECT_KEY"
        }[tokens[j].kind],
                tokens[j].root_index);
        if(tokens[j].kind == STRING || tokens[j].kind == NUMBER) {
            printf(", value: %s", tokens[j].address);
        }
        puts("");
    }

}