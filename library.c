#include "library.h"



void hello(void) {
    printf("Hello, World!\n");
}

bool found_true() {

}

bool found_false() {

}

bool found_null() {

}

char * encode_json(struct node node) {

    char * str = malloc(1);
    return str;
}

struct node * decode_json(const char * str) {
    enum states state = BEFORE_TOKEN;
    int index = 0;

    while (true) {
        char c = str[index++];
        if (state == BEFORE_TOKEN) {
            if (c == LEFT_SQUARE_BRACKET) {
                state = OPEN_ARRAY;
                continue;
            }
            else if (c == LEFT_CURLY_BRACKET) {
                state = OPEN_OBJECT;
                continue;
            }
            else if (c == literal_name_tokens[JSON_TRUE][0]) {
                state = PARSING_TRUE;
                continue;
            }
            else if (c == literal_name_tokens[JSON_FALSE][0]) {
                state = PARSING_FALSE;
                continue;
            }
            else if (c == literal_name_tokens[JSON_NULL][0]) {
                state = PARSING_NULL;
                continue;
            }
            else if (strchr(whitespace_tokens, c)) {
                continue;
            } else {
                json_error = JSONERR_INVALID_CHARACTER;
                return NULL;
            }
        }
        if (state == PARSING_TRUE || state == PARSING_FALSE || state == PARSING_NULL) {
            char * pos;
            char buffer[6] = { 0 };
            memcpy(buffer, &str[index], 5);
            if((pos = strstr(buffer, literal_name_tokens[JSON_TRUE])) != NULL && pos == buffer) {
                state = PARSING_TRUE;
                continue;
            }
            if((pos = strstr(buffer, literal_name_tokens[JSON_FALSE])) != NULL && pos == buffer) {
                state = PARSING_FALSE;
                continue;
            }
            if((pos = strstr(buffer, literal_name_tokens[JSON_NULL])) != NULL && pos == buffer) {
                state = PARSING_NULL;
                continue;
            }
        }

        break;
    }

    struct node * node = malloc(sizeof(struct node));

    return node;
}

void free_json(struct node * node) {
    free(node);
}

void free_json_str(char * json_str) {

    free(json_str);
}