#include "main.h"
#include "json.h"

char * testdata[] = {
    "true",
    "1",
    "123",
    "1203.4",
    "123.0",
    "123.05",
    "123.0e12",
    "-12.34e+25",
    "-12.34E+25",
    "-12.34E-25",
    "-12.34e25",
    "\"foo\"",
    "false",
    "null",
    " \" a random string\"",
    " \" a random string with \\u0000 correctly encoded null byte\"",
    "{\r\n\t "
        "\"foo\": \"bar\""
    "}",
    "[1, 2, \"foo\", true]",

    "{"
        "    \"Image\": {"
            "\"Width\":  800,"
            "\"Height\": 600,"
            "\"Title\":  \"View from 15th Floor\","
            "\"Thumbnail\":"
            "\"Url\":    {\"http://www.example.com/image/481989943\","
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
            "\"State\":     \"CA\","
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
    "]"
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
    "\"characters between 00 and 1F must use the unicode codepoint notation, \\u0000, not \1, \2, \0 .\"",
    "\"true and\" false",
    "0123", /* leading zeros are forbidden */
    ".12",
    "12.",
    "12.e10",
    "NaN",
    "Infinity",
    "0x12EF",
    "123E",
    "123e",
    "123.4e",
    "123.4E",
    "123.4e23e",
    "123.4e23.",
    "123.4e23,",
};

int main(int argc, char * argv[]) {

    printf("Hello!\n");
    for (int i = 0; i < sizeof(testdata) / sizeof(testdata[0]) ; i++) {
        json_error = JSON_ERROR_NO_ERRORS;
        struct json_parsed * json_parsed = decode_json(testdata[i], strlen(testdata[i]));
        printf("%s", encode_json(json_parsed));
        free_json(json_parsed);

        printf("For >>> %s <<<, \n -> %s\n", testdata[i], json_errors[json_error]);
    }

    puts("\n\n\n*** ALL SHOULD FAIL ***");

    for (int i = 0; i < sizeof(bogusjson) / sizeof(bogusjson[0]) ; i++) {
        json_error = JSON_ERROR_NO_ERRORS;
        struct json_parsed * json_parsed = decode_json(bogusjson[i], strlen(bogusjson[i]));
        printf("%s", encode_json(json_parsed));
        free_json(json_parsed);

        printf("For >>> %s <<<, \n -> %s\n", bogusjson[i], json_errors[json_error]);
    }

    (void)getchar();
    return 0;
}