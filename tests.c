#include "json.h"
#include <stdio.h>
#include <assert.h>

char * valid_json[] = {
    "true",
    "true  ",
    "  \t  true  ",
    "  \t  true",
    "10",
    "-0",
    "1",
    "123",
    "1203.4",
    "123.0",
    "123.05",
    "0.12",
    "-0.12",
    "-0.12e1",
    "-0e3",
    "-0e-3",
    "123.0e12",
    "12e0",
    "12e000000012",
    "12e+0",
    "12e-0",
    "12e1",
    "-12e1",
    "12e01",
    "12e+01",
    "-12e+01",
    "12e-01",
    "-12e-01",
    "-12.34e+25",
    "-12.34E+25",
    "-12.34E-25",
    "-12.34e25",
    "\"foo\"",
    "false",
    "null",
    " \" a random string\"",
    "[1, 2, \"foo\", [1, 2], 4  ]  ",
    "[1, 2, 3]",
    "[1, \"2\", 3, \"\", \"\", \"foo\"]",
    "[1 , 2 , 3 ]",
    "[1, 2, 3  ]",
    "[1, 2, 3  ]  ",
    " \" à random string é with lower block characters.\"",
//    " \" \u1011 random string with \u1011 correctly encoded null byte\"",
    " \" a random string with \\u0000 correctly encoded null byte.\"",
    "{\r\n\t "
        "\"foo\": \"bar\""
    "}",
    "{\"foo\": 1, \"bar\": \"foo\"}",
    "[1, 2, \"foo\", true]",
    "{"
        "    \"Image\": {"
            "\"Width\":  800,"
            "\"Height\": 600,"
            "\"Title\":  \"View from 15th Floor\","
            "\"Thumbnail\":"
                "{\"Url\": \"http://www.example.com/image/481989943\","
                "              \"Height\": 125,"
                "\"Width\":  \"100\""
            "},"
            "\"IDs\": [116, 943, 234, 38793]"
        "}"
    "}",

    "["
        "{"
            " \"precision\": \"zip\","
            "\"Latitude\":  37.7668,"
            "                    \"Longitude\": -122.3959,"
            "                    \"Address\":   \"\","
            "\"City\":      \"SAN FRANCISCO\","
            "\"State\":     [\"CA\", {\"foo\": [\"CA\", \"OK\"]}],"
            "\"Zip\":       \"94107\","
            "\"Country\":   \"US\""
        "},"
        "{"
            "\"precision\": \"zip\","
            "\"Latitude\":  37.371991,"
            "\"Longitude\": -122.026020,"
            "\"Address\":   \"\","
            "\"City\":      \"SUNNYVALE\","
            "\"State\":     \"CA\","
            "\"Zip\":       \"94085\","
            "\"Country\":   \"US\""
        "}"
    "]",
    "[[[0]]]",
    "[[[1, 3, [3, 5], 7]], 3]",
    "{\"foo\": 1, \"foo\": 1, \"foo\": 2, \"foo\": 1}",
    "\"no\\\\ \\\"white\tspace\"",
    "\"tést\"",
    "\"expect shortcuts \\\", \\\\, \\/, \b, \f, \n, \r, \r, \t  \"",
    "\"no shortcuts \a, \v, \' \047 \"",
//    "\"test 漫 \""
};

struct {
    char * str;
    size_t size;
} bin_safe_json[] = {
    {"\"fo\0o\"", sizeof("\"fo\0o\"")},
    {"\"\0foo\"", sizeof("\"\0foo\"")},
    {"\"foo\0\"", sizeof("\"foo\0\"")},
    {"\"\0fo\12o\"", sizeof("\"\0fo\12o\"")}
};

/* shouldn't be parsed as valid json */
char * bogus_json[] = {
    "",
    " ",
    "<!-- comment -->",
    "//comment",
    "\r\n\f ",
    "\r\n\f \"\r and \n are valid but \f is invalid whitespace\"",
    "NULL",
    "foo",
    "\1\2\3true\4\5\6",
    "true false",
    "true'",
    "true //comment",
    "true <!--comment -->",
    "true, false",
    "true,",
    "\"forget closing quote",
    "\"too many quotes\"\"",
    "'wrong quote'",
    "\"incomplete \\u122 unicode\"",
    "\"characters between 00 and 1F must use the unicode codepoint notation, \\u0000, not \1, \2, \0 .\"",
    "\"true and\" false",
    "0123",
    "123 345",
    "+0",
    "+1",
    "+12",
    "-",
    "12 .12",
    "12.&",
    "12. 12",
    "12 e",
    "12e 0",
    "12e+ 0",
    "12e +0",
    "12.4 e+0",
    "12.4 e +0",
    "--12",
    "-f12",
    "-1-2",
    "-12-.",
    "-12.-0",
    "-12.34e+25.2",
    "-12e12-2",
    "-12e++12-2",
    "-12e+-122",
    "-12e--12-2",
    ".12",
    "0.",
    "-0.",
    "0.e",
    "-0.E",
    "0.t",
    "-0.~",
    "0e",
    "-0e",
    "12.",
    "12..",
    "12..001",
    "12.001.",
    "12.3.",
    "12.e10",
    "NaN",
    "Infinity",
    "0x12EF",
    "123E",
    "123e",
    "123.4e",
    "123.4E",
    "0e+",
    "0e-",
    "123.4e23e",
    "123.4E23e",
    "123.4e23.",
    "123.4e23,",
    "12,3.4e23,",
    "123.4e,23,",
    "123.4e2,3,",
    "{true: 1}",
    "{false: 1}",
    "[false, 1] true",
    "[false, 1], true",
    "{false: 1}, true",
    "{false: 1, true}",
    "{null: 1}",
    "{2: 1}",
    "{[true]: 1}",
    "{{}: 1}",
    "{[]: 1}",
    "{\"test\"}",
    "{\"test\":}",
};

#ifdef WANT_JSON1
char * bogus_json1[] = {
    "true",
    "1",
    "[1, 2, 3]"
};
#endif

int main(void) {
    int i;

    puts("*** ALL SHOULD SUCCEED ***");

    for (i = 0; i < sizeof(valid_json) / sizeof(valid_json[0]) ; i++) {
        struct state state = {0};
        rjson((unsigned char*)valid_json[i], strlen(valid_json[i]), &state);
        printf("For >>> %s <<<, \n -> %s\n", valid_json[i], json_errors[state.error]);
        puts(print_debug(&state));
        puts(to_string(state.tokens_stack, state.token_cursor));
        fflush(stdout);
        assert(state.error == JSON_ERROR_NO_ERRORS);
    }

    puts("\n\n\n*** ALL SHOULD FAIL ***");

    for (i = 0; i < sizeof(bogus_json) / sizeof(bogus_json[0]) ; i++) {

        struct state state = {0};
        rjson((unsigned char*)bogus_json[i], strlen(bogus_json[i]), &state);
        printf("For >>> %s <<<, \n -> %s\n", bogus_json[i], json_errors[state.error]);
        puts(print_debug(&state));
        puts(to_string(state.tokens_stack, state.token_cursor));
        fflush(stdout);
        assert(state.error != JSON_ERROR_NO_ERRORS);
    }

    puts("\n\n\n*** BINARY SAFE ***");

    for (i = 0; i < sizeof(bin_safe_json) / sizeof(bin_safe_json[0]) ; i++) {

        struct state state = {0};
        rjson((unsigned char*)bin_safe_json[i].str, bin_safe_json[i].size, &state);
        printf("For >>> %s <<<, \n -> %s\n", bin_safe_json[i].str, json_errors[state.error]);
        puts(print_debug(&state));
        puts(to_string(state.tokens_stack, state.token_cursor));
        fflush(stdout);
        assert(state.error != JSON_ERROR_NO_ERRORS);
    }

#ifdef WANT_JSON1
    puts("\n\n\n*** SHOULD FAIL IN JSON1 MODE***");

    for (i = 0; i < sizeof(bogus_json1) / sizeof(bogus_json1[0]) ; i++) {

        struct state state = {.mode=JSON1};
        int res = rjson((unsigned char*)bogus_json1[i], strlen(bogus_json1[i]) - 1, &state);
        printf("For >>> %s <<<, \n -> %s\n", bogus_json1[i], json_errors[state.error]);
        puts(print_debug(&state));
        puts(to_string(*(state.tokens_stack), res));
        fflush(stdout);
        assert(state.error == JSON_ERROR_JSON1_ONLY_ASSOC_ROOT);
    }
#endif

    /* "[1, 2, \"foo\", [1, 2], 4  ]  ", */
    puts("\n\n\n*** EX NIHILO ***");
    struct state state = {0};
    PUSH_TOKEN(ROOT, NULL, &state);
    PUSH_TOKEN(ARRAY, NULL, &state);
    PUSH_ROOT(&state);
    START_STRING(&state);
    PUSH_STRING(&state, "1", 1);
    PUSH_STRING_TOKEN(NUMBER, &state);
    START_STRING(&state);
    PUSH_STRING(&state, "2", 1);
    PUSH_STRING_TOKEN(NUMBER, &state);
    START_STRING(&state);
    PUSH_STRING(&state, "foo", 3);
    PUSH_STRING_TOKEN(STRING, &state);
    PUSH_TOKEN(ARRAY, NULL, &state);
    PUSH_ROOT(&state);
    START_STRING(&state);
    PUSH_STRING(&state, "1", 1);
    PUSH_STRING_TOKEN(NUMBER, &state);
    START_STRING(&state);
    PUSH_STRING(&state, "2", 1);
    PUSH_STRING_TOKEN(NUMBER, &state);
    CLOSE_ROOT(&state);
    START_STRING(&state);
    PUSH_STRING(&state, "4", 1);
    PUSH_STRING_TOKEN(NUMBER, &state);
    puts(to_string(state.tokens_stack, state.token_cursor));

    puts("\n\n\n*** EZ JSON ***");
    assert(strcmp(serialize(tokenize("42")), "42") == 0);
    puts("OK");
    puts(serialize(tokenize("[1, 2, {\"4\": 5, \"4\": 6, \"4\": 5 }]")));

    return 0;
}
