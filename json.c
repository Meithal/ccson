#include "json.h"

enum json_errors json_error;

char * encode_json(struct json_parsed *json_parsed) {

    return "";
}

/**
 * Parses a string of JSON
 * @param str
 * @param length
 * @return
 */
struct json_parsed * decode_json(const char *str, unsigned int length) {
    enum states state = BEFORE_TOKEN;
    int index = 0;
    int current_root = 0;

    struct json_parsed * json_parsed = malloc(sizeof(struct json_parsed));
    memset(json_parsed, 0, sizeof(struct json_parsed));

    while (true) {
        char c = str[index];

        if (c == '\0') {
            break;
        }

        if (state == BEFORE_TOKEN) {
            if (c == structs[LEFT_SQUARE_BRACKET]) {
                state = OPEN_ARRAY;
                continue;
            }
            else if (c == structs[LEFT_CURLY_BRACKET]) {
                state = OPEN_OBJECT;
                continue;
            }
            else if (c == lits[JSON_TRUE][0]) {
                state = PARSING_TRUE;
                continue;
            }
            else if (c == lits[JSON_FALSE][0]) {
                state = PARSING_FALSE;
                continue;
            }
            else if (c == lits[JSON_NULL][0]) {
                state = PARSING_NULL;
                continue;
            }
            else if (strchr(whsps, c)) {
                index += 1;
                continue;
            }
            else if (strchr(digit_starters, c)) {
                int len = json_parse_number(&str[index], length - index);
                if (!json_error) {
                    json_parsed = push_node(json_number, current_root, &JSON_TRUE_SINGLETON, json_parsed);
                    if (json_parsed == NULL) {
                        goto panic;
                    }
                    index += len;
                    continue;
                } else {
                    goto panic;
                }
            }
            else if(c == '+') {
                json_error = JSON_ERROR_LEADING_PLUS;
                goto panic;
            }
            else {
                json_error = JSON_ERROR_INVALID_CHARACTER;
                goto panic;
            }
        }

        if (state == PARSING_TRUE || state == PARSING_FALSE || state == PARSING_NULL) {
            if (state == PARSING_TRUE) {
                if (length - index < strlen(lits[JSON_TRUE])) {
                    json_error = JSON_ERROR_JSON_TOO_SHORT;
                    goto panic;
                }
            }
            if (state == PARSING_FALSE) {
                if (length - index < strlen(lits[JSON_FALSE])) {
                    json_error = JSON_ERROR_JSON_TOO_SHORT;
                    goto panic;
                }
            }
            if (state == PARSING_NULL) {
                if (length - index < strlen(lits[JSON_NULL])) {
                    json_error = JSON_ERROR_JSON_TOO_SHORT;
                    goto panic;
                }
            }
            /* we are guaranteed that the string is long enough to hold at least 'true', 'false', ... */
            bool found = false;
            if (state == PARSING_TRUE) {
                if (strncmp(&str[index], lits[JSON_TRUE], strlen(lits[JSON_TRUE])) == 0) {
                    json_parsed = push_node(json_true, current_root, &JSON_TRUE_SINGLETON, json_parsed);
                    index += strlen(lits[JSON_TRUE]);
                    found = true;
                }
            }
            if (state == PARSING_FALSE) {
                if (strncmp(&str[index], lits[JSON_FALSE], strlen(lits[JSON_FALSE])) == 0) {
                    json_parsed = push_node(json_false, current_root, &JSON_FALSE_SINGLETON, json_parsed);
                    index += strlen(lits[JSON_FALSE]);
                    found = true;
                }
            }
            if (state == PARSING_NULL) {
                if (strncmp(&str[index], lits[JSON_NULL], strlen(lits[JSON_NULL])) == 0) {
                    json_parsed = push_node(json_null, current_root, &JSON_NULL_SINGLETON, json_parsed);
                    index += strlen(lits[JSON_NULL]);
                    found = true;
                }
            }

            if (json_error) {
                goto panic;
            }

            if (!found) {
                index += 1;
            }

            continue;
        }

        break;
    }

    if (!json_semcheck(json_parsed)) {
        goto panic;
    }

    if (false) {
        panic:
        free_json(json_parsed);
        return NULL;
    }

    return json_parsed;
}

/**
 * Parses the str, returns the number of characters parsed.
 * Sets json_error if the number is wrongly formatted.
 * @param str
 * @param len
 * @return
 */
int json_parse_number(const char *str, int len) {
    int i = 0;

    struct states {
        bool parsing_first_character;
        bool right_after_leading_minus;
        bool must_find_dot_or_exp_or_end;
        bool leading_minus_found;
        bool leading_zero_found;
        bool leading_digits_found;
        bool first_digit_found;
        bool decimal_dot_found;
        bool right_after_decimal_dot;
        bool exponent_found;
        bool right_after_exponent;
        bool plus_minus_after_exponent_found;
        bool digit_after_exponent_found;
    } state = { .parsing_first_character = true };

    while (true) {
        char c = str[i++];

        if (c == '\0') {

            if (state.right_after_exponent) {
                json_error = JSON_ERROR_EXP_EMPTY;
            }

            if (state.exponent_found && !state.digit_after_exponent_found) {
                json_error = JSON_ERROR_EXP_NO_DIGIT;
            }

            if(state.right_after_decimal_dot) {
                json_error = JSON_ERROR_MUST_HAVE_DIGIT_AFTER_DOT;
            }
            return i - 1;
        }


        /* leading */

        if (state.must_find_dot_or_exp_or_end) {

            if (!strchr(".eE", c)) {
                json_error = JSON_ERROR_MUST_FIND_DOT_OR_EXP;
                return -1;
            }
        }

        if (state.right_after_leading_minus) {
            if(c == '0') {
                state.must_find_dot_or_exp_or_end = true;
            } else if (strchr(digits19, c)) {
                ;
            } else {
                json_error = JSON_ERROR_WRONG_POST_MINUS;
                return -1;
            }

            state.right_after_leading_minus = false;
        }

        if (state.right_after_leading_minus || state.parsing_first_character) {
            if(c == '0') {
                state.must_find_dot_or_exp_or_end = true;
            }
        }

//        if (!state.decimal_dot_found)

        /* leading / */

        /* minus */
        if (c == '-' && !state.parsing_first_character && !state.right_after_exponent) {
            json_error = JSON_ERROR_WRONG_MINUS;
            return -1;
        }
        if (c == '-') {
            state.leading_minus_found = true;
            state.right_after_leading_minus = true;
        }
        /* minus / */

        /* decimal */

        if (c == '.' && !state.leading_digits_found) {
            json_error = JSON_ERROR_NO_LEADING_DIGIT;
            return -1;
        }

        if (strchr(digits, c)) {
            state.leading_digits_found = true;
        }

        if (state.right_after_decimal_dot) {
            if (!strchr(digits, c)) {
                json_error = JSON_ERROR_MUST_HAVE_DIGIT_AFTER_DOT;
                return -1;
            }
        }

        if (c == '.' && state.decimal_dot_found) {
            json_error = JSON_ERROR_TOO_MANY_DOTS;
            return -1;
        }

        if (state.right_after_decimal_dot) {
            state.right_after_decimal_dot = false;
        }

        if (c == '.') {
            state.decimal_dot_found = true;
            state.right_after_decimal_dot = true;
        }

        /* decimal / */

        /* exponent part */
        if (state.exponent_found) {
            if (strchr(digits, c)) {
                state.digit_after_exponent_found = true;
            }

            if (state.plus_minus_after_exponent_found && !strchr(digits, c)) {
                json_error = JSON_ERROR_NUMBER_WRONG_CHAR;
            }
        }

        if (state.right_after_exponent) {
            if (c == '+' || c == '-' || strchr(digits, c)) {
                state.right_after_exponent = false;
                state.plus_minus_after_exponent_found = true;
            } else {
                json_error = JSON_ERROR_WRONG_POST_EXP;
                return -1;
            }
        }

        if (strchr("Ee", c) && state.exponent_found) {
            json_error = JSON_ERROR_TOO_MANY_EXPONENTS;
            return -1;
        }

        if (strchr("Ee", c)) {
            state.exponent_found = true;
            state.right_after_exponent = true;
        }
        /* exponent part / */

        /* must be at the end */
        if (state.parsing_first_character) {
            state.parsing_first_character = false;
        }

        if (!strchr(digit_glyps, c)) {
            break;
        }

    }

    return i;
}

struct json_parsed *push_node(enum json_value_kind kind, int parent, size_t *address, struct json_parsed *json_parsed) {
    if (json_parsed->values_count && json_parsed->values[json_parsed->values_count].parent == parent) {
        json_error = JSON_ERROR_TWO_OBJECTS_HAVE_SAME_PARENT;
        return NULL;
    }

    struct json_value node = {
        .kind = kind,
        .address = address,
        .parent = parent
    };

    json_parsed->values_count++;
    realloc(json_parsed, sizeof(struct json_parsed) + json_parsed->values_count * sizeof(json_parsed->values[0]));
    json_parsed->values[json_parsed->values_count - 1] = node;

    return json_parsed;
}

void free_json(struct json_parsed *json_parsed) {
    free(json_parsed);
}

void free_json_str(char * json_str) {

    free(json_str);
}

/**
 * Checks the parsed json follows json format rules
 * @param json_parsed
 * @return
 */
bool json_semcheck(struct json_parsed *json_parsed) {
    bool valid = true;

    if(json_parsed->values_count == 0) {
        json_error = JSON_ERROR_EMPTY;
        valid = false;
    }

    return valid;
}
