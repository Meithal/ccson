//
// Created on 15/01/2020.
//

#include "json.h"
#include "assert.h"
#define peek_at(where) string[state->ordinal + where]
#define OK 0

size_t len_whitespace(char* string, struct state * state) {
    size_t count = 0;
    while(strchr(whitespaces, string[state->ordinal + count++]) != NULL);
    return count;
}

void start_string() {
    string_cursor += string_pool[string_cursor];
}

void push_string(char* string, size_t length) {
    assert(length > 0);
    memcpy(string_pool + string_cursor + string_pool[string_cursor], string, length);
    string_pool[string_cursor] += length;
}

int rjson(char* string, struct state* state) {
    size_t total_length = strlen(string);
    if (state->ordinal == 0) {
        state->error = JSON_ERROR_NO_ERRORS;
        state->kind = WHITESPACE_BEFORE_VALUE;
    }

    while(true) {

        if (state->ordinal > total_length) {
            return JSON_ERROR_NO_ERRORS;
        }

        if (state->error != JSON_ERROR_NO_ERRORS) {
            return state->error;
        }

        size_t remaining = strlen(string + state->ordinal);
        size_t to_inc = 0;
        switch (state->kind) {
            case WHITESPACE_BEFORE_VALUE: {
                state->kind = EXPECT_VALUE;
                to_inc = len_whitespace(string, state);
                break;
            }

            case WHITESPACE_AFTER_VALUE: {
                state->kind = AFTER_VALUE;
                to_inc = len_whitespace(string, state);
                break;
            }

            case EXPECT_VALUE:
            {
                if (strncmp(string + state->ordinal, "true", remaining) == 0) {
                    tokens[token_cursor++] = (struct token) {.kind=TRUE, .root=roots[root_cursor], .address=&JSON_TRUE_SINGLETON};
                    to_inc = strlen("true");
                    state->kind = WHITESPACE_AFTER_VALUE;
                }
                else if (strncmp(string + state->ordinal, "false", remaining) == 0) {
                    tokens[token_cursor++] = (struct token) {.kind=FALSE, .root=roots[root_cursor], .address=&JSON_FALSE_SINGLETON};
                    to_inc = strlen("false");
                    state->kind = WHITESPACE_AFTER_VALUE;
                }
                else if (strncmp(string + state->ordinal, "null", remaining) == 0) {
                    tokens[token_cursor++] = (struct token) {.kind=JNULL, .root=roots[root_cursor], .address=&JSON_NULL_SINGLETON};
                    to_inc = strlen("null");
                    state->kind = WHITESPACE_AFTER_VALUE;
                }
                else if (peek_at(0) == '"') {
                    start_string();
                    state->kind = FOUND_OPEN_QUOTE;
                    to_inc = 1;
                }
                else if (peek_at(0) == '[') {
                    roots[++root_cursor] = &arrays_pool[array_cursor++];
                    tokens[token_cursor++] = (struct token) {.kind=ARRAY, .root=roots[root_cursor], .address=roots[root_cursor]};
                    state->kind = OPEN_ARRAY;
                    to_inc = 1;
                }
                else if (peek_at(0) == '{') {
                    roots[++root_cursor] = &assoc_pool[assoc_cursor++];
                    tokens[token_cursor++] = (struct token) {.kind=OBJECT, .root=roots[root_cursor], .address=roots[root_cursor]};
                    state->kind = OPEN_ASSOC;
                    to_inc = 1;
                }
                else if (strchr(digit_starters, peek_at(0)) != NULL) {
                    start_string();
                    state->kind = START_NUMBER;
                    to_inc = 1;
                }
                else {
                    state->error = JSON_ERROR_EMPTY;
                }

                break;
            }
            case AFTER_VALUE:
            {
                if (root_cursor == 0) {
                    return OK;
                }
                if(((struct token*)roots[root_cursor])->kind == ARRAY) {
                    state->kind = ARRAY_AFTER_VALUE;
                    to_inc = 0;
                }
                if(((struct token*)roots[root_cursor])->kind == OBJECT) {
                    state->kind = ASSOC_AFTER_VALUE;
                    to_inc = 0;
                }

                break;
            }
            case OPEN_ARRAY: {
                if (peek_at(len_whitespace(string, state)) == ']') {
                    root_cursor--;
                    state->kind = WHITESPACE_AFTER_VALUE;
                    to_inc = len_whitespace(string, state) + 1;
                } else {
                    state->kind = WHITESPACE_BEFORE_VALUE;
                    to_inc = 0;
                }

                break;
            }
            case OPEN_ASSOC: {
                if (peek_at(len_whitespace(string, state)) == '}') {
                    root_cursor--;
                    state->kind = WHITESPACE_AFTER_VALUE;
                    to_inc = len_whitespace(string, state) + 1;
                }
                break;
            }
            case FOUND_OPEN_QUOTE:
            {
                if (peek_at(0) == '\\') {
                    state->kind = LITERAL_ESCAPE;
                    to_inc = 1;
                } else if (peek_at(0) == '"') {
                    tokens[token_cursor++] = (struct token) {.kind=STRING, .address=string_pool + 0, .root=roots[root_cursor]};
                    state->kind = WHITESPACE_AFTER_VALUE;
                    to_inc = 1;
                } else {
                    state->kind = IN_STRING;
                    to_inc = 1;
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
                state->kind = IN_STRING;
                to_inc = 1;

                break;
            }
            case IN_STRING: {
                if (peek_at(0) == '\\') {
                    state->kind = LITERAL_ESCAPE;
                    to_inc = 1;
                } else if (peek_at(0) == '"') {
                    tokens[token_cursor++] = (struct token) {.kind=STRING, .address=string_pool + string_cursor, .root=roots[root_cursor]};
                    state->kind = WHITESPACE_AFTER_VALUE;
                    to_inc = 1;
                } else {
                    push_string((char[]){peek_at(0)}, 1);
                    state->kind = IN_STRING;
                    to_inc = 1;
                }

                break;
            }

            case START_NUMBER: {
                if (peek_at(0) == '-') {
                    state->kind = NUMBER_AFTER_MINUS;
                    to_inc = 1;
                } else {
                    state->kind = NUMBER_AFTER_MINUS;
                    to_inc = 0;
                }

                break;
            }
            case NUMBER_AFTER_MINUS:
            {
                if (peek_at(0) == '0') {
                    state->kind = EXPECT_FRACTION;
                    to_inc = 1;
                } else if (strchr(digits19, peek_at(0)) != NULL) {
                    push_string((char[]){peek_at(0)}, 1);
                    state->kind = IN_NUMBER;
                    to_inc = 1;
                } else {
                    state->error = JSON_ERROR_INVALID_NUMBER;
                }

                break;
            }
            case IN_NUMBER: {
                if (strchr(digits19, peek_at(0)) != NULL) {
                    push_string((char[]){peek_at(0)}, 1);
                    state->kind = IN_NUMBER;
                    to_inc = 1;
                } else {
                    state->kind = EXPECT_FRACTION;
                    to_inc = 0;
                }

                break;
            }
            case EXPECT_FRACTION: {
                if (peek_at(0) == '.') {
                    push_string((char[]){peek_at(0)}, 1);
                    state->kind = IN_FRACTION;
                    to_inc = 1;
                } else {
                    state->kind = EXPECT_EXPONENT;
                    to_inc = 0;
                }

                break;
            }
            case EXPECT_EXPONENT: {
                if (peek_at(0) == 'e' || peek_at(0) == 'E') {
                    push_string((char[]){peek_at(0)}, 1);
                    state->kind = EXPONENT_EXPECT_PLUS_MINUS;
                    to_inc = 0;
                }

                break;
            }
            case IN_FRACTION: {
                if (strchr(digits, peek_at(0))) {
                    push_string((char[]){peek_at(0)}, 1);
                    state->kind = IN_FRACTION_DIGIT;
                    to_inc = 1;
                } else {
                    state->error = JSON_ERROR_INVALID_NUMBER;
                }

                break;
            }
            case IN_FRACTION_DIGIT: {
                if (strchr(digits, peek_at(0))) {
                    push_string((char[]){peek_at(0)}, 1);
                    state->kind = IN_FRACTION_DIGIT;
                    to_inc = 1;
                } else {
                    state->kind = EXPECT_EXPONENT;
                    to_inc = 0;
                }

                break;
            }
            case EXPONENT_EXPECT_PLUS_MINUS: {
                if (peek_at(0) == '+' || peek_at(0) == '-') {
                    push_string((char[]){peek_at(0)}, 1);
                    state->kind = EXPECT_EXPONENT_DIGIT;
                    to_inc = 1;
                } else {
                    state->kind = EXPECT_EXPONENT_DIGIT;
                    to_inc = 0;
                }

                break;
            }
            case EXPECT_EXPONENT_DIGIT: {
                if (strchr(digits, peek_at(0))) {
                    push_string((char[]){peek_at(0)}, 1);
                    state->kind = IN_EXPONENT_DIGIT;
                    to_inc = 1;
                } else {
                    state->error = JSON_ERROR_INVALID_NUMBER;
                }

                break;
            }
            case IN_EXPONENT_DIGIT: {
                if (strchr(digits, peek_at(0))) {
                    push_string((char[]) {peek_at(0)}, 1);
                    state->kind = IN_EXPONENT_DIGIT;
                    to_inc = 1;
                } else {
                    tokens[token_cursor++] = (struct token) {.kind=NUMBER, .address=string_pool + string_cursor, .root=roots[root_cursor]};
                    state->kind = WHITESPACE_AFTER_VALUE;
                    to_inc = 0;
                }

                break;
            }
            case ARRAY_AFTER_VALUE: {
                if(peek_at(0) == ',') {
                    state->kind = WHITESPACE_BEFORE_VALUE;
                    to_inc = 1;
                } else if(peek_at(0) == ']') {
                    root_cursor--;
                    state->kind = WHITESPACE_AFTER_VALUE;
                    to_inc = 1;
                } else {
                    state->error = JSON_ERROR_INVALID_CHARACTER_IN_ARRAY;
                }

                break;
            }
            case ASSOC_AFTER_VALUE:
                break;
            case ASSOC_WHITESPACE_BEFORE_KEY: {
                state->kind = EXPECT_VALUE;
                to_inc = len_whitespace(string, state);

                break;
            }
        }

        state->ordinal += to_inc;
    }
#undef peek_at
#undef OK
}
