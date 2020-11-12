#include "../json.h"
#include "tests_profile.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

struct {
    char const * str;
    char const * ref;
} valid_json[] = {
    "true", "true",                           /* 0 */
    "true  ", "true",                         /* 1 */
    "  \t  true  ", "true",                   /* 2 */
    "  \t  true", "true",                     /* 3 */
    "10", "10",                               /* 4 */
    "-0", "-0",                               /* 5 */
    "1", "1",                                 /* 6 */
    "123", "123",                             /* 7 */
    "1203.4", "1203.4",                       /* 8 */
    "123.0", "123.0",                         /* 9 */
    "123.05", "123.05",                       /* 10 */
    "0.12", "0.12",                           /* 11 */
    "-0.12", "-0.12",                         /* 12 */
    "-0.12e1", "-0.12e1",                     /* 13 */
    "-0e3", "-0e3",                           /* 14 */
    "-0e-3", "-0e-3",                         /* 15 */
    "123.0e12", "123.0e12",                   /* 16 */
    "12e0", "12e0",                           /* 17 */
    "12e000000012", "12e000000012",           /* 18 */
    "12e000000000", "12e000000000",           /* 19 */
    "12e+0", "12e+0",                         /* 20 */
    "12e-0", "12e-0",                         /* 21 */
    "12e1", "12e1",                           /* 22 */
    "-12e1", "-12e1",                         /* 23 */
    "12e01", "12e01",                         /* 24 */
    "12e+01", "12e+01",                       /* 25 */
    "-12e+01", "-12e+01",                     /* 26 */
    "12e-01", "12e-01",                       /* 27 */
    "-12e-01", "-12e-01",                     /* 28 */
    "-12.34e+25", "-12.34e+25",               /* 29 */
    "-12.34E+25", "-12.34E+25",               /* 30 */
    "-12.34E-25", "-12.34E-25",               /* 31 */
    "-12.34e25", "-12.34e25",                 /* 32 */
    "\"foo\"", "\"foo\"",                     /* 33 */
    "false", "false",                         /* 34 */
    "null", "null",                           /* 35 */
    " \" a random string\"", "\" a random string\"",                           /* 36 */
    "[1, 2, \"foo\", [1, 2], 4  ]  ", "[1,2,\"foo\",[1,2],4]",                 /* 37 */
    "[1, 2, 3]", "[1,2,3]",                                                    /* 38 */
    "[1, \"2\", 3, \"\", \"\", \"foo\"]", "[1,\"2\",3,\"\",\"\",\"foo\"]",     /* 39 */
    "[1 , 2 , 3 ]", "[1,2,3]",                                                 /* 40 */
    "[1, 2, 3  ]", "[1,2,3]",                                                  /* 41 */
    "[1, 2, 3  ]  ", "[1,2,3]",                                                /* 42 */
    " \" à random string é with lower block characters.\"", "\" à random string é with lower block characters.\"",                          /* 43 */
    " \" \xCD\xBF random unicode string\"", "___",                                                           /* 44 */
    " \" a random string with \\u0000 correctly encoded null byte.\"", "\" a random string with \\u0000 correctly encoded null byte.\"",    /* 45 */
    "{\r\n\t "                                                          /* 46 */
        "\"foo\": \"bar\""
    "}", "{\"foo\":\"bar\"}",
    "{\"foo\": 1, \"bar\": \"foo\"}", "{\"foo\":1,\"bar\":\"foo\"}",    /* 47 */
    "[1, 2, \"foo\", true]", "[1,2,\"foo\",true]",                      /* 48 */
    "{"                           /* 0 */
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
    "}", "___",

    "["                           /* 49 */
        "{"
            " \"precision\": \"zip\","
            "\"Latitude\":  37.7668,"
            "                    \"Longitude\": -122.3959,"
            "                    \"Address\"\t:   \"\",\t"
            "\"City\"  :      \"SAN FRANCISCO\","
            "\"State\":     [\"CA\", {\"foo\": [\"CA\", \"OK\"]}]  ,"
            "\"Zip\":       \"94107\","
            "\"Country\":\"US\""
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
    "]", "___",                           /* 50 */
    "[[[0]]]", "[[[0]]]",                 /* 51 */
    "[[[1, 3, [3, 5], 7]], 3]", "[[[1,3,[3,5],7]],3]",                                                 /* 52 */
    "{\"foo\": 1, \"foo\": 1, \"foo\": 2, \"foo\": 1}", "{\"foo\":1,\"foo\":1,\"foo\":2,\"foo\":1}",   /* 53 */
    "\"no\\\\ \\\"white\tspace\"", "\"no\\\\ \\\"white\\tspace\"",                                     /* 54 */
    "\"tést\"", "\"tést\"",                                                                            /* 55 */
    "\"expect shortcuts \\\", \\\\, \\/, \b, \f, \n, \r, \t  \"", "\"expect shortcuts \\\", \\\\, /, \\b, \\f, \\n, \\r, \\t  \"",   /* 56 */
    "\"test 漫 \"", "\"test 漫 \"",                                                                         /* 58 */
    "\"\xFF\"", (char[]){'\"', '\xFF', '\xFD', '\"', '\0'},  /* invalid utf is replaced by \\uFFFD */      /* 59 */
    "\"\xAF\"", (char[]){'\"', '\xFF', '\xFD', '\"', '\0'},                                                /* 60 */
    "12310000000000000033432.2E324342423423423224234234", "12310000000000000033432.2E324342423423423224234234",  /* 61 */
};

struct {
    char const * str;
    size_t size;
} bin_safe_json[] = {
    {"\"fo\0o\"", sizeof("\"fo\0o\"") -1},
    {"\"\0foo\"", sizeof("\"\0foo\"") -1},
    {"\"foo\0\"", sizeof("\"foo\0\"") -1},
    {"\"\0fo\12o\"", sizeof("\"\0fo\12o\"") -1}
};

/* shouldn't be parsed as valid json */
char const * bogus_json[] = {
    "",
    " ",
    "<!-- comment -->",
    "//comment",
    "\r\n\f ",
    "\r\n\f \"\r and \n are valid but \f is invalid whitespace\"",
    "NULL",
    "nUll",
    "nul",
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
    "{\"test\":1}}",
    "[[[0]]]]",
    "[[[0]}]",
    "[0,[]",
    "[0,[],",
    "\"no shortcuts \a, \v, \' \047 \"", "___",
    "[1, 2, 3,]",
    "[1, 2,, 3,]",
    "[1, 2, , 3,]",
    "{,}",
    "{\"a\",2}",
    "{\"a\":2,}",
    "{\"a\" : 2 ,}",
    "{\"a\" : ,2 }",
    "{\"a\" : 2, \"b\" }",
    "{\"a\" : 2,, \"b\" }",
    "{\"a\" : 2 , , \"b\" }",
    "{\"a\" : 2 ,\"b\", }",
    "{\"a\" : 2 ,\"b\":, }",
    "{\"a\" : 2 ,\"b\": 3, }",
    "\"no shortcuts \a, \v, \' \047 \"", "___",
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

    long long frequency = start_profiler();
    long long total_parse_time = 0;
    long long total_write_time = 0;
    printf("Timer frequency: %lld by second.\n", frequency);

    for (i = 0; i < sizeof(valid_json) / sizeof(valid_json[0]) ; i++) {
        struct state state = { 0 };
        long long start = start_timer();
        enum json_errors error = rjson(cs_strlen(valid_json[i].str), (unsigned char*)valid_json[i].str, &state);
        long long end = elapsed(start, frequency);
        total_parse_time += end;
        printf("%d: For >>> %s <<<, elapsed: %lld\n -> %s\n", i, valid_json[i].str, end, json_errors[error]);
        printf("%s", print_debug(&state.tokens));
        start = start_timer();
        char* out = to_string(&state.tokens);
        end = elapsed(start, frequency);
        total_write_time += end;
        puts(out);
        fflush(stdout);
        assert(error == JSON_ERROR_NO_ERRORS);
        if(strcmp(valid_json[i].ref, "___") != 0) {
            assert(strcmp(to_string_compact(&state.tokens), valid_json[i].ref) == 0);
        }
    }

    puts("\n\n\n*** ALL SHOULD FAIL ***");

    for (i = 0; i < sizeof(bogus_json) / sizeof(bogus_json[0]) ; i++) {

        struct state state = { 0 };
        long long start = start_timer();
        enum json_errors error = rjson(cs_strlen(bogus_json[i]), (unsigned char*)bogus_json[i], &state);
        long long end = elapsed(start, frequency);
        total_parse_time += end;
        printf("%d: For >>> %s <<<, elapsed: %lld\n -> %s\n", i, bogus_json[i], end, json_errors[error]);
        puts(print_debug(&state.tokens));
        start = start_timer();
        char* out = to_string(&state.tokens);
        end = elapsed(start, frequency);
        total_write_time += end;
        puts(out);
        fflush(stdout);
        assert(error != JSON_ERROR_NO_ERRORS);
    }

    puts("\n\n\n*** BINARY SAFE ***");

    for (i = 0; i < sizeof(bin_safe_json) / sizeof(bin_safe_json[0]) ; i++) {

        struct state state = { 0 };
        long long start = start_timer();
        enum json_errors error = rjson(bin_safe_json[i].size, (unsigned char*)bin_safe_json[i].str, &state);
        long long end = elapsed(start, frequency);
        total_parse_time += end;
        printf("%d: For >>> %s <<<, elapsed: %lld\n -> %s\n", i, bin_safe_json[i].str, end, json_errors[error]);
        puts(print_debug(&state.tokens));
        start = start_timer();
        char* out = to_string(&state.tokens);
        end = elapsed(start, frequency);
        total_write_time += end;
        puts(out);
        fflush(stdout);
        assert(error == JSON_ERROR_UNESCAPED_CONTROL);
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
    memset(static_stack, 0, sizeof(static_stack));
    memset(static_pool, 0, sizeof(static_pool));
    state.tokens.tokens_stack = static_stack;
    state.copies.string_pool = static_pool;

    START_AND_PUSH_TOKEN(&state, ROOT, "#custom root");
    START_AND_PUSH_TOKEN(&state, ARRAY, "[");
    PUSH_ROOT(&state);
    START_AND_PUSH_TOKEN(&state, NUMBER, "1");
    START_AND_PUSH_TOKEN(&state, NUMBER, "2");
    START_AND_PUSH_TOKEN(&state, STRING, "\"foo\"");
    START_AND_PUSH_TOKEN(&state, ARRAY, "[");
    PUSH_ROOT(&state);
    START_AND_PUSH_TOKEN(&state, NUMBER, "1");
    START_AND_PUSH_TOKEN(&state, NUMBER, "2");
    CLOSE_ROOT(&state);
    START_AND_PUSH_TOKEN(&state, NUMBER, "4");
    puts(to_string(&state.tokens));
    puts(to_string_compact(&state.tokens));
    assert(strcmp(to_string_compact(&state.tokens),"[1,2,\"foo\",[1,2],4]") == 0);

#ifndef HAS_VLA
    printf("Total parsing time: %lld\n", total_parse_time);
    printf("Total Writing time: %lld\n", total_write_time);
    FILE* file;
    int error = fopen_s(&file, "profile_data.txt", "a");
    if(error != 0) {
        char out[80] = { 0 };
        _strerror_s(out, 80, "err");
        puts(out);
    }
    fprintf(
            file,
            "Total parsing time: %lld ; Writing: %lld ; %s %s - %s\n",
            total_parse_time,
            total_write_time,
            LAST_COMMIT_HASH, LAST_COMMIT_COUNT, CMAKE_GENERATOR);
    fclose(file);
#endif

    return 0;
}
