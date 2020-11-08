# Cisson
"C-(J)SON" aka JSON library in C language.


## How to use
```C
#include "json_single_header.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

char * valid_json[] = {
    "true",
    "true  ",
    "[1, 2, 3]",
    "  \t  true",
    "0",
    "-0",
}

int main(void) {
    int i;

    puts("*** ALL SHOULD SUCCEED ***");

    for (i = 0; i < sizeof valid_json / sizeof *valid_json) ; i++) {
        struct state state = {0};
        int res = rjson(valid_json[i], strlen(valid_json[i]), &state);
        printf("For >>> %s <<<, \n -> %s\n", valid_json[i], json_errors[state.error]);
        puts(print_debug(&state));
        puts(to_string(state.tokens_stack, res));
        assert(state.error == JSON_ERROR_NO_ERRORS);
    }
    return 0;

}

```


## Sturdy
Parsing is single pass and doesn't involve recursion so no risk of 
stack busting from untrusted input. 
Covered by almost 200 tests (for both valid and invalid cases). 
Doesn't use dynamic memory.
By default uses a small pool of static memory that should 
cover most use cases. The size can be customized or disabled completely 
if you want to supply it your own buffers (from eg. malloc).

Strings are binary safe. Even if null characters (`\0`) are not allowed
in JSON documents, the size of strings is stored apart. The numbers are also
 stored as strings, it is let to the user to translate them
 to natives values using `strtod` or a big numbers library.


## Minimal footprint
Written in ANSI C (tests and stringifier are in C99). 
Compiles by default into a shared library (DLL, .so). Can be used 
header only, no LibC. Should be cross platform (not tested).

On failure the parser returns the number of successfully parsed tokens
and a specific error message. That lets you to fix your entry stream
before trying again.


## Reentrant
You can produce a json document over the course of several days
and be able to probe it and store it as it is being produced, without errors.
That makes it easy to work with a sliding window in a limited memory
environment.

Every stored token is guaranteed to be complete and can be concatenated 
with subsequent outputs to produce a valid document.


## Binary safe
This library is binary safe.


## Fast
By default memory fragmentation doesn't happen as every parsed token
is concatenated contiguously. Memory is written only once and never
 moved.

If you maintain an external hashmap of fully qualified identifiers 
 (with a scripting language for example) you can get 
 O(1) accesses in every case. However fundamentally JSON doesn't 
 require object keys to be unique so a query with a fqn doesn't 
 make a lot of sense.

Unlike [JSMN](https://github.com/zserge/jsmn), this library does a full 
copy of strings and numbers.
This makes reentry easier as you can clear the original buffer
between passes, in cost of some speed. 
It will also convert control characters in their escaped 
unicode form (`\0` becomes `\\u0000`, `\1` becomes `\\u0001`, etc.).


## JSON 1 and 2
Supports JSON 1 and 2.


## utf 8, 16><, 32><
Has basic utf8 support (only first 128 + 1,920 codepoints).
Doesn't support extended ASCII. Doesn't support UTF16 nor UTF32.


## Bugs
Report them on https://gitlab.com/Meithal/json/-/issues
