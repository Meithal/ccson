Cisson is a JSON library in C. It can serialize C objects
into JSON and parse JSON into C objects.

## Requirements
Tested on Windows and Linux, should compile with MSVC and GCC.
Cmake is helpful to easily include this library in your project,
but not required if you use the single-header version of it.
No dependency, including LibC.

## Install
Used single-header
```c
#define CISSON_IMPLEMENTATION
#include "cisson.h"
```
`cisson.h` is the only file you have to compile. 
***
Used as a static library, through Cmake:
```cmake
add_subdirectory(cisson)
add_executable(my_exe my_source.c my_header.h ...) # those are your project files
target_link_libraries(sjson my_exe) # sjson is the static lib target, xjson is the dyn lib one
```
You don't have to define `CISSON_IMPLEMENTATION` in this case 
before including `cisson.h`,
the implementation will be provided by the library.

## Usage
Having a C object like this
```c
struct foo = {
        .foo = "bar",
        .array = {1, 2, 3},
        .question = true
};
```
we can turn it into JSON text like that

```c
#define CISSON_IMPLEMENTATION
#include "cisson.h"
#include <stdio.h> 

int main(void) {
    struct foo = {
        .foo = "bar",
        .array = {1, 2, 3},
        .question = true
    };
    
    char vals[3][10] = { 0 };
    sprintf(vals[0], "%d", foo.array[0]);
    sprintf(vals[1], "%d", foo.array[1]);
    sprintf(vals[2], "%d", foo.array[2]);
    
    struct cisson_state state = {0};
    
    start_state(&state, static_stack, sizeof static_stack,
        static_pool, sizeof static_pool);
    
    START_AND_PUSH_TOKEN(&state, ROOT, "#custom root");
    PUSH_ROOT(&state);
    START_AND_PUSH_TOKEN(&state, OBJECT, "{");
    PUSH_ROOT(&state);
    START_AND_PUSH_TOKEN(&state, STRING, "\"foo\"");
    PUSH_ROOT(&state);
    START_AND_PUSH_TOKEN(&state, STRING, foo.foo);
    CLOSE_ROOT(&state);
    START_AND_PUSH_TOKEN(&state, STRING, "\"array\"");
    PUSH_ROOT(&state);
    START_AND_PUSH_TOKEN(&state, ARRAY, "[");
    PUSH_ROOT(&state);
    START_AND_PUSH_TOKEN(&state, NUMBER, vals[0]);
    START_AND_PUSH_TOKEN(&state, NUMBER, vals[1]);
    START_AND_PUSH_TOKEN(&state, NUMBER, vals[2]);
    CLOSE_ROOT(&state);
    CLOSE_ROOT(&state);
    START_AND_PUSH_TOKEN(&state, STRING, "\"question\"");
    PUSH_ROOT(&state);
    START_AND_PUSH_TOKEN(&state, foo.question ? TRUE : FALSE, foo.question ? "true" : "false");
    puts(to_string(&state.tokens)); /* {"foo":"bar","array":[1,2,4],"question":true} */
}
```
Every token shall be bound to a root. The state keeps
in memory what the current root is, to allow the resuming of
the building of the JSON at any time. Array elements have as root
their array, object properties have as root their object, and
the values associated to properties have as root that 
property. When we call `PUSH_ROOT`, we change the root
the next tokens will have their root as. When calling
`CLOSE_ROOT`, the current root will become what the root of the 
current root was, hence going back in the
hierarchy of roots by one notch.

Cisson only accepts string values, you must convert your non-string
values before they can be tokenized, by using `sprintf` for example.

`to_string` outputs a cisson object as raw JSON.

With `push_token`, we can rewrite the previous code like this
```c
#include "json.h"

int main() {
    struct cisson_state state = {0};
    
    start_state(&state, static_stack, sizeof static_stack,
                static_pool, sizeof static_pool);

    push_token(&state, "#smart root"); /* only the leading # is necessary to signal a document root */
    push_token(&state, "{");
    push_token(&state, "\"foo\"");
    push_token(&state, ":");
    push_token(&state, "\"bar\"");
    push_token(&state, ">");
    push_token(&state, "\"array\"");
    push_token(&state, ":");
    push_token(&state, "[");
    push_token(&state, "1");
    push_token(&state, "2");
    push_token(&state, "4");
    push_token(&state, ">>");
    push_token(&state, "\"question\"");
    push_token(&state, ":");
    push_token(&state, "true");
    puts(to_string_compact(&state.tokens)); /* {"foo":"bar","array":[1,2,4],"question":true} */

}
```
The `>` symbol closes a root and replaces `CLOSE_ROOT`.
`PUSH_ROOT` is called on the following tokens: `{`, `[`, `:`, `#`.
We don't have to provide the nature of the token we want to push,
as it can be guessed by the first character of the string.

`stream_tokens_` can be used to compress the previous code even further.

```c
#include "json.h"

int main() {
    struct cisson_state state = {0};
    
    start_state(&state, static_stack, sizeof static_stack,
                static_pool, sizeof static_pool);

    stream_tokens(&state, '~',
        &(char[]){"#smart root~{~\"foo\"~:~\"bar\"~>~\"array\"~:~[~1~2~4~>>~\"question\"~:~true"});
    puts(to_string_compact(&state.tokens)); /* {"foo":"bar","array":[1,2,4],"question":true} */

}
```

We provide a separator character to use, and a stream of
tokens separated by the separator. The stream must be writeable.

You can mix `stream_tokens_`, `push_tokens` and macros to
build a JSON object.

***

To convert the JSON `{"foo":"bar","array":[1,2,4],"question":true}`
into a C object, we can do

```c
#define CISSON_IMPLEMENTATION
#include "cisson.h"
#include <stdio.h> 
#include <string.h> 

int main(void) {
    struct foo {
        char * foo;
        int array[3];
        bool question;
    };
    
    struct foo foo = { 0 };
    
    char * json = "{\"foo\":\"bar\",\"array\":[1,2,4],\"question\":true}"
            
    struct cisson_state state;
    struct token stack[0x200];
    char pool[0x200];
    start_state(&state, stack, sizeof stack,
        pool, sizeof pool);

    enum json_errors error = rjson(strlen(json), json, &state);
    
    if (error != JSON_ERROR_NO_ERRORS) {
        puts(json_errors[error]);
        return 1;
    }
    
    foo.foo = query(&state, "/foo").address;
    sscanf(query(&state, "/array/0").address, "%d", &foo.array[0]);
    sscanf(query(&state, "/array/1").address, "%d", &foo.array[1]);
    sscanf(query(&state, "/array/2").address, "%d", &foo.array[2]);
    foo.question = query(&state, "/array/2").kind == TRUE ? true: false;
    
    assert(memcmp((struct foo){.foo = foo.foo, .array={1, 2, 4}, .question=true}, foo, sizeof foo) == 0);
}
```

`rjson` reads raw JSON and converts it to a cson object.

`query` uses a JSON pointer (RFC 6901) to fetch a token from 
the JSON tree. A token has an `.address` property that 
points to the string associated with the token. It also
has a `.kind` property.

We used our own pool of memory instead of using the 
shared static one, since our object will point to it, and we don't
want to lose the value.