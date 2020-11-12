# Cisson - C language JSON library

"C-(J)SON" aka binary safe, single header, fast and correct, 
dependency free JSON library in C language. 


## How to use

Copy the json_single_header.h file in your project then
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
        struct state state = {.cursor=(unsigned char*)valid_json[i].str};
        rjson(cs_strlen(valid_json[i]), &state);
        puts(to_string(&state.tokens));
        assert(state.error == JSON_ERROR_NO_ERRORS);
    }
    return 0;

}

```

If you want to compile it as a separate library or a runtime
library you need [Cmake](https://cmake.org/) executable 
accessible in you windows `PATH`. From the root folder type

```
mkdir build
cd build
cmake ..
cmake --build . --config Release
./Release/tests[.exe]
```


## Minimal footprint

Comes as a single header file and doesn't have any dependency
(even on stdlib). Logic is under 400 loc.
Can be tinkered for more advanced uses such as a static library, 
a runtime library (DLL), to use true stdlib functions, to
run tests and profiling.

## Conform

Today the best JSON library is considered to be 
[RapidJSON](https://github.com/Tencent/rapidjson). The fastest
library is [SIMD Json](https://github.com/simdjson/simdjson).
Both are in C++, the later uses SIMD assembly instructions
as its name implies.

Benchmarks for JSON conformance can be found on [rapidJSON
project](https://github.com/miloyip/nativejson-benchmark),
or [JSONTestSuite](https://github.com/nst/JSONTestSuite).

It appears that the only C library with more than 80%
conformance today is
[ccan/json](https://github.com/rustyrussell/ccan/)

Compared to other libraries in C, this library 
is more than 90% conforming and possibly the most
conforming C library. It mostly lacks UTF16 and UTF32 support
as it tries to focus on UTF8 for now.

## Resilient

All the parser needs is a pointer to a string containing valid
JSON. On error it will simply return the last error status.
At pointer + 0 sits bogus JSON. 
Parsing is single pass and will never look back.
When it has to look forth and can't,
it will return; which means you can't fragment your JSON
between true/false/null tokens of between unicode escape
points (`\uXXXX`). That's a known limit.

To allow reentry, you can optionally pass in a `state` structure
pointer that will keep the current switch state the parser was in, you 
can reuse this pointer to "resume" parsing in the state the
parser was previously stopped. If you don't the state will be
held in a global static variable that is not thread safe.

To know where the parser will write all the tokens
it finds you can provide it a pointer
to an array of `token`s. If you don't it will use a global
static array that can hold up to 8000 tokens - that value can be
customized at compilation. 

If you use your own custom buffer you have to make sure 
it is clean; if you use the default one the parser will
clean it automatically on each "fresh" switch state.

To allow this library to work with binary safe strings 
you must provide their size in a separate parameter. 
A macro function does that for you
by simply calling `strlen()` on your cursor. 
If you have to work with binary 
unsafe string you want to NOT use the macro function, but provide
the actual size either from `sizeof(string) - 1` or from
an upper protocol layer, aka a `Content-Length:` field or similar.

To sum up the minimum requirement to parse JSON with this
lib is a pointer to a utf8 string.

## Robust

No recursion is involved, so no risk of 
stack busting from untrusted input.

200 tests cover both valid and invalid cases. 

No dynamic memory is used, instead work is done in a small 
pool of static memory that should cover most use cases. 
Its size can be customized or disabled completely 
if you want to supply it your own buffers (from eg. malloc).

Strings are binary write safe and read safe. 
Even if null characters (`\0`) are not allowed
in JSON documents, the size of strings is stored apart. 
The numbers are also stored as strings: it is let to the user to translate them
to natives values using `strtod` or a big numbers library.

The JSON standard doesn't impose any limit on how big 
a number can be but the stored number will be semantically
correct and can be consumed for example by the 
`int()` function of python without error or loss.

## Fast
By default memory fragmentation doesn't happen as every parsed token
is concatenated contiguously. Memory is written only once and never
 moved.

If you maintain an external hashmap of fully qualified identifiers 
(with a scripting language for example) you can get 
O(1) accesses in every case. However fundamentally JSON doesn't 
require object keys to be unique so a query with a fqn doesn't 
make a lot of sense.

## Non tampering

Data is not tampered with, specifically a JSON object of this shape
`{
 "foo": "a",
 "foo": "a",
 "foo": 1,
 "foo": "a"
}`
 will be printed as
```
 {
   "foo": "a",
   "foo": "a",
   "foo": 1,
   "foo": "a"
 }
```

A number that cannot be represented on a machine, will not 
print NaN or a value equal to +/-INT_MAX, but will keep
the initial significant digits intact, no rounding, 
no truncation happens.

By design, parsed elements keep the order in which
they are parsed, and inserting data has to be done
by the user through `memmove` semantics.

You may use empty placeholders if you expect to insert data
later in the middle of your DOM, as empty nodes are simply 
skipped during printing.

## *Comparability*
  
[RFC8259](https://tools.ietf.org/html/rfc8259#section-8.3)
mentions that one should be able to "compare
two JSON strings for equality". 
To aid this task this library provides a `minified_string`
function that tries to find the shortest shape a string
can have: 
`"\/"` becomes `"/"`; any escaped unicode is 
replaced by its actual encoded unicode,
except if a shorter representation exists.

UTF codepoints are not converted to 
shorter canonical representations.
This is a known limitation.

Numbers could be shortened too, for example
`00000000001.000000e00` may be shortened to `1`, 
`100000` to `1e5`, `-0` replaced by `0`, and so on.
But considered that the RFC mentions nothing about that matter, and 
that ambiguous cases such as `100` and `1e2` exist, we keep 
digits exactly how they are input.

If you keep those limitations in mind, you may use
`minified_string()` + `strcmp()` to compare two 
JSON strings for "equality".

## Reentrant
You can produce a json document over the course of several 
days and be able to probe it and store it as it is being 
produced, without errors.
That makes it easy to work with a sliding window in a 
limited memory environment.

Some tokens cannot be reentered, namely truncated 
`true`/`false`/`null` tokens, truncated `\uXXXX` escaped
code points in strings and truncated BOMs.

## Binary safe
This library is binary safe in both ways.

## Thread safe
You can use this library in a thread-safe way.

## JSON 1, 2 and >
Supports JSON 1 and 2, but also tries to follow RFC8259 guidelines.
Np support for JSON 5.

## UTF 8, 16><, 32><
Accepts UTF8 as long as it conforms to JSON syntax, 
aka control characters must be escaped.
Doesn't support extended ASCII. 
Doesn't support UTF16 nor UTF32. 

## Bugs
Report them on https://gitlab.com/Meithal/cisson/-/issues

## Some reads
* [json.org](https://www.json.org/)
* [Parsing JSON is a Minefield 💣](http://seriot.ch/parsing_json.php)
* Test suite: 
[JSONTestSuite](https://github.com/nst/JSONTestSuite)
* Other test suite: 
[nativejson-benchmark](https://github.com/miloyip/nativejson-benchmark)