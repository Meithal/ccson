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
            if (c == structurals[LEFT_SQUARE_BRACKET]) {
                state = OPEN_ARRAY;
                continue;
            }
            else if (c == structurals[LEFT_CURLY_BRACKET]) {
                state = OPEN_OBJECT;
                continue;
            }
            else if (c == litterals[JSON_TRUE][0]) {
                state = PARSING_TRUE;
                continue;
            }
            else if (c == litterals[JSON_FALSE][0]) {
                state = PARSING_FALSE;
                continue;
            }
            else if (c == litterals[JSON_NULL][0]) {
                state = PARSING_NULL;
                continue;
            }
            else if (strchr(whitespaces, c)) {
                index += 1;
                continue;
            } else {
                json_error = JSON_ERROR_INVALID_CHARACTER;
                goto panic;
            }
        }

        if (state == PARSING_TRUE || state == PARSING_FALSE || state == PARSING_NULL) {
            if (state == PARSING_TRUE) {
                if (length - index < strlen(litterals[JSON_TRUE])) {
                    json_error = JSON_ERROR_JSON_TOO_SHORT;
                    goto panic;
                }
            }
            if (state == PARSING_FALSE) {
                if (length - index < strlen(litterals[JSON_FALSE])) {
                    json_error = JSON_ERROR_JSON_TOO_SHORT;
                    goto panic;
                }
            }
            if (state == PARSING_NULL) {
                if (length - index < strlen(litterals[JSON_NULL])) {
                    json_error = JSON_ERROR_JSON_TOO_SHORT;
                    goto panic;
                }
            }
            /* we are guaranteed that the string is long enough to hold at least 'true', 'false', ... */
            bool found = false;
            if (state == PARSING_TRUE) {
                if (strncmp(&str[index], litterals[JSON_TRUE], strlen(litterals[JSON_TRUE])) == 0) {
                    json_parsed = push_node(json_true, current_root, json_parsed);
                    index += strlen(litterals[JSON_TRUE]);
                    found = true;
                }
            }
            if (state == PARSING_FALSE) {
                if (strncmp(&str[index], litterals[JSON_FALSE], strlen(litterals[JSON_FALSE])) == 0) {
                    json_parsed = push_node(json_false, current_root, json_parsed);
                    index += strlen(litterals[JSON_FALSE]);
                    found = true;
                }
            }
            if (state == PARSING_NULL) {
                if (strncmp(&str[index], litterals[JSON_NULL], strlen(litterals[JSON_NULL])) == 0) {
                    json_parsed = push_node(json_null, current_root, json_parsed);
                    index += strlen(litterals[JSON_NULL]);
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

    if (!json_check_validity(json_parsed)) {
        goto panic;
    }

    if (false) {
        panic:
        free_json(json_parsed);
        return NULL;
    }

    return json_parsed;
}

struct json_parsed * push_node(enum json_value_kind kind, int parent, struct json_parsed *json_parsed) {
    if (json_parsed->values_count && json_parsed->values[json_parsed->values_count].parent == parent) {
        json_error = JSON_ERROR_TWO_OBJECTS_HAVE_SAME_PARENT;
        return NULL;
    }

    struct json_value node = {
        .kind = kind,
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
 * Checks the validity of a parsed json text
 * TODO: fill up an array of parsing errors.
 * @param json_parsed
 * @return
 */
bool json_check_validity(struct json_parsed * json_parsed) {
    bool valid = true;

    if(json_parsed->values_count == 0) {
        json_error = JSON_ERROR_EMPTY;
        valid = false;
    }

    return valid;
}
