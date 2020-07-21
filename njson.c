//
// Created on 15/01/2020.
//

#include "json.h"

enum {INTERTOKEN};

struct json_parsed * ndecode_json(const char *str, unsigned int length) {
    struct json_parsed * stack = calloc(1, sizeof(struct json_parsed));

    enum new_state state = INTERTOKEN;
}
