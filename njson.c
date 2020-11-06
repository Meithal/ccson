#include "json.h"

int static len_whitespace(char* string, struct state * state) {
    int count = 0;
    while(string[state->ordinal + count] != '\0' && strchr(whitespaces, string[state->ordinal + count]) != NULL) {
        count++;
    }
    return count;
}

void static start_string() {
    string_cursor += (int)string_pool[string_cursor-1] + 1;
}

void static push_string(char* string, int length) {
    assert(length > 0);
    memcpy(string_pool + string_cursor + string_pool[string_cursor-1], string, length);
    string_pool[string_cursor-1] += length;
}

void static close_root(struct state * state) {
    state->root_index = tokens[state->root_index].root_index;
}

void static push_root(struct state * state) {
    state->root_index = state->token_cursor - 1;
}

void static push_token(enum kind kind, void * address, struct state * state) {
    tokens[state->token_cursor++] = (struct token) {.kind=kind, .root_index=state->root_index, .address=address};
}

static inline size_t remaining(char * string, struct state* state) {
    return strlen(string + state->ordinal);
}

static inline int in(char* hay, char needle) {
    return needle != '\0' && strchr(hay, needle);
}


EXPORT int rjson(char* string, struct state* state, void** tokens__) {

#define peek_at(where) string[state->ordinal + where]
#define set_state_and_advance_by(which_, advance_) state->kind = which_; state->ordinal += advance_; to_inc = 0

    *tokens__ = tokens;

    memcpy(tokens, (struct token [0x200]){(struct token) {.kind=ROOT}}, sizeof (struct token[0x200]));
    state->token_cursor = 1;
    memset(string_pool, 0, sizeof string_pool);
    string_cursor = 0;

    // todo: make fully reentrant
    // todo: make ANSI/STDC compatible
    // fixme: handle unicode literals
    // todo: make libc optional
    // fixme: longer than 64 bytes strings
    // fixme: create tree from nothing

    if (state->ordinal == 0) {
        state->error = JSON_ERROR_NO_ERRORS;
        state->kind = WHITESPACE_BEFORE_VALUE;
    }

    for(;;) {

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
                    push_token(TRUE, NULL, state);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, strlen("true"));
                }
                else if (strncmp("false", string + state->ordinal, strlen("false")) == 0) {
                    push_token(FALSE, NULL, state);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, strlen("false"));
                }
                else if (strncmp("null", string + state->ordinal, strlen("null")) == 0) {
                    push_token(JSON_NULL, NULL, state);
                    set_state_and_advance_by(WHITESPACE_AFTER_VALUE, strlen("null"));
                }
                else if (peek_at(0) == '"') {
                    start_string();
                    set_state_and_advance_by(FOUND_OPEN_QUOTE, 1);
                }
                else if (peek_at(0) == '[') {
                    push_token(ARRAY, &tokens[state->root_index], state);
                    push_root(state);
                    set_state_and_advance_by(OPEN_ARRAY, 1);
                }
                else if (peek_at(0) == '{') {
                    push_token(OBJECT, &tokens[state->root_index], state);
                    push_root(state);
                    set_state_and_advance_by(OPEN_ASSOC, 1);
                }
                else if (in(digit_starters, peek_at(0))) {
                    start_string();
                    set_state_and_advance_by(START_NUMBER, 0);
                }
                else if (tokens[state->root_index].kind != ROOT) {
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
                if (tokens[state->root_index].kind == ROOT) {
                    if (remaining(string, state)) {
                        state->error = JSON_ERROR_NO_SIBLINGS;
#ifdef WANT_JSON1
                    } else if(state->mode == JSON1 && tokens[state->token_cursor-1].kind != OBJECT) {
                        state->error = JSON_ERROR_JSON1_ONLY_ASSOC_ROOT;
#endif
                    } else {
                        return state->token_cursor;
                    }
                }
                else if(tokens[state->root_index].kind == ARRAY) {
                    set_state_and_advance_by(ARRAY_AFTER_VALUE, 0);
                }
                else if(tokens[state->root_index].kind == OBJECT) {
                    set_state_and_advance_by(ASSOC_AFTER_VALUE, 0);
                }
                else if(tokens[state->root_index].kind == STRING) {
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
                    push_token(NUMBER, string_pool + string_cursor, state);
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
                    push_token(NUMBER, string_pool + string_cursor, state);
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
                push_token(STRING, string_pool + string_cursor, state);
                if (tokens[state->root_index].kind == OBJECT) {
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

        assert(to_inc != -999);
    }
#undef peek_at
#undef set_state_and_advance_by
}

#ifdef HAS_VLA
void print_debug(struct state * state) {
    int j;
    for (j = 0; j < state->token_cursor; ++j) {
        printf("%d: kind: %s, root: %d", j, (char*[]){
                "UNSET", "ROOT", "TRUE", "FALSE", "JSON_NULL",
                "STRING", "NUMBER", "ARRAY", "OBJECT", "OBJECT_KEY"
        }[tokens[j].kind],
                tokens[j].root_index);
        if(tokens[j].kind == STRING || tokens[j].kind == NUMBER) {
            char dest[*((char*)tokens[j].address-1) + 1];
            dest[sizeof(dest) - 1] = '\0';
            printf(", value: %s", memcpy(dest, tokens[j].address, *((char*)tokens[j].address-1)));
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


char * to_string(struct token tokens_[0x200], int max) {
    // todo: make the caller handle the buffer

    static char output[0x1600];
    memset(output, 0, sizeof output);
    int cursor = 0;
    int ident = 0;
    int j;
    for (j = 1; j < max; ++j) {
        if (tokens_[tokens_[j].root_index].kind == STRING) {
            cursor += sprintf(output + cursor, ": ");
        } else {
            cursor += sprintf(output + cursor, "%s", print_ident(ident));
        }

        if (tokens_[j].kind == TRUE) cursor += sprintf(output + cursor, "true");
        if (tokens_[j].kind == FALSE) cursor += sprintf(output + cursor, "false");
        if (tokens_[j].kind == JSON_NULL) cursor += sprintf(output + cursor, "null");
        if (tokens_[j].kind == STRING || tokens_[j].kind == NUMBER) {
            char dest[*((char *) tokens_[j].address - 1) + 1];
            dest[sizeof dest - 1] = '\0';
            cursor += sprintf(
                    output + cursor, (char *[]) {"\"%s\"", "%s"}[tokens_[j].kind == NUMBER],
                    memcpy(dest, tokens_[j].address, *((char *) tokens_[j].address - 1))
            );
        }
        if (tokens_[j].kind == ARRAY) cursor += sprintf(output + cursor, "[\n"), ident++;
        if (tokens_[j].kind == OBJECT) cursor += sprintf(output + cursor, "{\n"), ident++;
        if (j <= max) {
            if (tokens_[j + 1].root_index < tokens_[j].root_index) {
                if (j + 1 == max
                || tokens_[j + 1].root_index != tokens_[tokens_[j].root_index].root_index
                || tokens_[tokens_[j + 1].root_index].kind == ARRAY) {
                    int target = tokens_[j + 1].root_index;
                    int cur_node = j;
                    for (;;) {

                        if(tokens_[tokens_[cur_node].root_index].kind == ARRAY) {
                            cursor += sprintf(output + cursor, "\n%s]", print_ident(--ident));
                        }
                        else if(tokens_[tokens_[cur_node].root_index].kind == OBJECT) {
                            cursor += sprintf(output + cursor, "\n%s}", print_ident(--ident));
                        }
                        if(tokens_[(cur_node = tokens_[cur_node].root_index)].root_index == target) {
                            break;
                        }
                    }
                }
            }
            if (j + 1 < max && (
                tokens_[tokens_[j].root_index].kind == STRING || tokens_[tokens_[j].root_index].kind == ARRAY
            ) && tokens_[j].kind != ARRAY && tokens_[j].kind != OBJECT) {
                cursor += sprintf(output + cursor, ",\n");
            }
        }
    }
    snprintf(output + cursor, 1, "");
    return output;
}
#endif // HAS_VLA
