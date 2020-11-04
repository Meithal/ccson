#include "main.h"
#include "json.h"
#include <assert.h>

char * valid_json[] = {
    "true",
    "true  ",
    "  \t  true  ",
    "  \t  true",
    "0",
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
    "[1, 2, \"azerty\", [1, 2], 4  ]  ",
    "[1, 2, 3]",
    "[1, \"2\", 3, \"\", \"\", \"foo\"]",
    "[1 , 2 , 3 ]",
    "[1, 2, 3  ]",
    "[1, 2, 3  ]  ",
//    " \" a random string with \\u0000 correctly encoded null byte\"",
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
    "[[[1, 3, [3, 5], 7]], 3]"
};

/* shouldn't be parsed as valid json */
char * bogusjson[] = {
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

int main(void) {

    for (int i = 0; i < sizeof(valid_json) / sizeof(valid_json[0]) ; i++) {
        struct state state = {0};
        rjson(valid_json[i], &state);
        printf("For >>> %s <<<, \n -> %s\n", valid_json[i], json_errors[state.error]);
        print_debug();
        fflush(stdout);
        assert(state.error == JSON_ERROR_NO_ERRORS);
    }

    puts("\n\n\n*** ALL SHOULD FAIL ***");

    for (int i = 0; i < sizeof(bogusjson) / sizeof(bogusjson[0]) ; i++) {

        struct state state = {0};
        rjson(bogusjson[i], &state);
        printf("For >>> %s <<<, \n -> %s\n", bogusjson[i], json_errors[state.error]);
        print_debug();
        fflush(stdout);
        assert(state.error != JSON_ERROR_NO_ERRORS);
    }

    return 0;
}