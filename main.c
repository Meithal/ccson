#include "main.h"
#include "library.h"

char * testdata[] = {
"-12.34e+25",
"\"foo\"",
"true",
"false",
"null",
" \" a random string\"",
" \" a random string with \0null byte\"",
" \" a random string with \u0000 correctly encoded null byte\"",
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


int main(int argc, char * argv[]) {

    printf("Hello!\n");
    json_error = JSONERR_NO_ERRORS;
    printf("%s", encode_json(*decode_json(testdata[0])));

    return 0;
}