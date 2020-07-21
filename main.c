#include "main.h"
#include "json.h"
#include <stdalign.h>

char * valid_json[] = {
    "true",
    "0",
    "-0",
    "1",
    "123",
    "1203.4",
    "123.0",
    "123.05",
    "123.0e12",
    "12e0",
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

(bar) = 12;

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
    "+0",
    "+1",
    "+12",
    "--12",
    "-f12",
    "-1-2",
    "-12-.",
    "-12.-0",
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
};

int *swap(int a, int b) {
    static int res[2];
    res[0] = b;
    res[1] = a;
    return (int *) &res;
}

int main(int argc, char * argv[], char* env[]) {
//    for (int i = 0; i <10 ; i++) {
//        printf("%s\n", env[i]);
//    }
//
//    return 0;
//    int * res = swap(1, 2);
//    printf("%d, %d\n", res[0], res[1]);
//    printf("%d, %d, %d, %d\n", 1 - 2, 1u -2, 1u - 2u, 1 - 2u);
//
//
//  char a  = 'a';
//  printf("%zu, %zu \n", sizeof(a), sizeof('a'));
//    printf("%lf\n", 0.0/0);
//    printf("%lf\n", strtod("nan", NULL));
//    printf("%x`\n", 0.0/0);
//    printf("%x\n", strtod("nan", NULL));
//  return 0;
//    char *test[] = {
//            "",
//            "test123",
//            "12",
//            "9999",
//            "10000",
//            "12toto23",
//            "  45674"
//    };
//    int rec;
//
//    for (int i = 0; i < sizeof(test) / sizeof(*test) ; i++) {
//        sscanf(test[i], "%4d", &rec);
//        printf("%04d\n", rec);
//    }
//    FILE * tmp = tmpfile();
//    stdin = tmp;
//    return 0;
//printf("%zu", sizeof(_Bool));
//return 0;
//    int a = 1;
//    if(a) {} else {puts("boo");}
//    FILE * tmp = tmpfile();
//    fputs("jacques Dupont 22 boulanger#Paris\n", tmp);
//    fputs("Jean Louis Dupont 42 representant de commerce#La fosse a l'eau\n", tmp);
//
//
//    char rec[256];
//    fseek(tmp, 0, SEEK_SET);
//    char nom[256];
//    int addresse;
//    char profession[246];
//    char lieu[256];
//    while (!feof(tmp) && !ferror(tmp)) {
//        char * ret1 = fgets(rec, 256, tmp);
//        if(ret1 == NULL) break;
//        sscanf(rec, "%[^0-9]%d %[^#]#%[^\n]\n", nom, &addresse, profession, lieu);
//        printf("Nom: %s\naddresse: %d\nProfession: %s\nLieu: %s\n---\n", nom, addresse, profession, lieu);
//    }
//    fclose(tmp);
//
//    return 0;
//    char hello[] = "hello\0world\0does it work though";
//    int acc = 0;
////    printf("%s%n%s%n%s\n", hello, &res, hello+res, &res, hello+res);
//    printf("%s", hello + (acc+=printf("%s", hello+(acc+=printf("%s", hello) + 1)) + 1));
//    return 0;
//    _Bool foo = 0;
//    for(int i = 0; i < 10;i++) {
//        printf("%s\n", foo++ ? "true" : "false");
//    }
//    for(int i = 0; i < 10;i++) {
//        printf("%s\n", foo-- ? "true" : "false");
//    }

//    int toto[] = {1, 2, 3};
//    printf("%d, %d, %d", toto[0], toto[1], toto[2]);
//    memcpy(toto, (int[]){3, 2, 1}, sizeof(toto));
//    printf("%d, %d, %d", toto[0], toto[1], toto[2]);
//    char c;
//    while ((c = getchar()) != EOF) c++;
//unsigned long a = 23;
//signed char b = -23;
//
//printf( "a %c b\n", a < b ? '<' : (a == b ? '=' : '>') );

//    int i = 0, j = 0, k=0, c=3;
//    a: 0;
//    int foo[3] = {i++};
//    printf("foo: %d, %p\n", foo[0], foo);
//    if(c-- >0) {
//        goto a;
//    }
//    printf("count: %d\n", i);
//    struct bar{int a;} *bar;
//    b: 0;
//    bar = &((struct bar){j++});
//    printf("bar: %d, %p\n", bar->a, &bar);
//    if(c++ <2) {
//        goto b;
//    }
//    printf("count: %d\n", j);
//    c: 0;
//    printf("baz: %p, %p\n", &((struct bar){k++})), &((struct bar){k++});
//    if(c-- >0) {
//        goto c;
//    }
//    printf("count: %d\n", k);
//
//    struct s {int i;} *p = 0, *q;
//    int l = 0;
//    again:
//    q = p, p = &((struct s){ l++ });
//    if (l < 3) goto again; // note; if a loop were used, it would end scope here,
//    // which would terminate the lifetime of the compound literal
//    // leaving p as a dangling pointer
//    printf("cppref %d, %d", p == q, q->i);
//
//    return 0;

//    int foo[] = {[0] = 3, [2] = foo[0] + 2};
    printf("%zu %zu\n", sizeof(struct {int toto;char tata; char titi;}), alignof(struct {int toto;char tata; char titi;}));
    printf("%zu %zu\n", sizeof(struct {int alignas(16) toto;char alignas(16) tata ; char titi;}), alignof(struct {int alignas(16) toto;char alignas(16) tata ; char titi;}));
    printf("int %zu\n", sizeof(int));
    printf("%zu\n", sizeof(struct {short s1 ;
        short s2 ;
        int x ;}));
    printf("%zu\n", sizeof(struct {short s1 ;
        int x ;
        short s2 ;}));

    return 0;
    for (int i = 0; i < sizeof(valid_json) / sizeof(valid_json[0]) ; i++) {
        json_error = JSON_ERROR_NO_ERRORS;
        struct json_parsed * json_parsed = decode_json(valid_json[i], strlen(valid_json[i]));
        printf("%s", encode_json(json_parsed));

        free_json(json_parsed);

        printf("For >>> %s <<<, \n -> %s\n", valid_json[i], json_errors[json_error]);
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