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
    "[1, 2, 3]",
    "0",
    "-0",
}

int main(void) {
    int i;

    puts("*** ALL SHOULD SUCCEED ***");

    for (i = 0; i < sizeof valid_json / sizeof *valid_json) ; i++) {
        struct state state = {.cursor=(unsigned char*)bin_safe_json[i].str};
        rjson(cs_strlen(valid_json[i]), &state);
        puts(to_string(&state.tokens));
        assert(state.error == JSON_ERROR_NO_ERRORS);
    }
    return 0;

}

```


## Resilient
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
header only without any dependencies (even stdlib).


## Reentrant
You can produce a json document over the course of several days
and be able to probe it and store it as it is being produced, without errors.
That makes it easy to work with a sliding window in a limited memory
environment.

Every stored token is guaranteed to be complete and can be concatenated 
with subsequent outputs to produce a valid document.

Some tokens cannot be reentered, namely truncated 
`true`/`false`/`null` tokens, truncated `\uXXXX` escaped
code points in strings and truncated BOMs.


## Binary safe
This library is binary safe in both ways (reads both unescaped
NULs and encodes them safely).


## Fast
By default memory fragmentation doesn't happen as every parsed token
is concatenated contiguously. Memory is written only once and never
 moved.

If you maintain an external hashmap of fully qualified identifiers 
 (with a scripting language for example) you can get 
 O(1) accesses in every case. However fundamentally JSON doesn't 
 require object keys to be unique so a query with a fqn doesn't 
 make a lot of sense.


## JSON 1, 2 and >
Supports JSON 1 and 2, but also tries to follow RFC8259 guidelines.


## UTF 8, 16><, 32><
Supports UTF8.
Doesn't support extended ASCII. Doesn't support UTF16 nor UTF32.


## Bugs
Report them on https://gitlab.com/Meithal/json/-/issues
