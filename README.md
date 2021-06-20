Cisson is a JSON library in C. It serializes C objects
into JSON and parses JSON into C objects.

## Requirements
Tested with GCC and MSVC. Cmake is optional.

## Install
Used single-header
```c
#define CISSON_IMPLEMENTATION
#include "cisson.h"
```
The preprocessor will copy `cisson.h` in your file, and
there is nothing more to do on your side.
***
Used as a static library, through Cmake:
```cmake
add_subdirectory(cisson)
add_executable(my_exe my_source.c my_header.h ...) # those are your project files
target_link_libraries(sjson my_exe) # sjson is the static lib target, xjson is the dyn lib one
```
In this case, you don't have to define `CISSON_IMPLEMENTATION` 
before including `cisson.h`,
the library already has the implementation.

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
Every token has a root that they bind to. The state keeps
in memory what the current root is, for example if the current root
is an array, every token we push will be a value of this array. 
This allows to resume the building of the JSON tree at any time. 

Array elements have as root their array; 
Object properties have as root their object; 
The values associated to object properties have as root that 
very property. When we call `PUSH_ROOT`, we change the root
the next tokens will have their root as. When calling
`CLOSE_ROOT`, the current root will become what the root of the 
current root was, hence going back in the
hierarchy of roots by one notch.

Cisson only accepts string values, you must convert your non-string
values before they can be tokenized; by using `sprintf` for example.

`to_string` converts your cisson object into a JSON string
and returns a pointer to it.

With `push_token`, we can rewrite the previous code like this
```c
#include "json.h"

int main() {
    struct cisson_state state = {0};
    
    start_state(&state, static_stack, sizeof static_stack,
                static_pool, sizeof static_pool);
    /* cisson has a static stack of tokens and a static pool
     * of characters where the token contents will be copied to.
     * You can use your own stack and pool if you plan to 
     * have more than one cisson object in memory at any time. */

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

`stream_tokens` can be used to compress the previous code even further.

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

You can mix `stream_tokens`, `push_tokens` and macros to
build a JSON object.

***

To convert the JSON `{"foo":"bar","array":[1,2,4],"question":true}`
into a C object,

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

`query` uses a JSON pointer (see [RFC 6901](https://datatracker.ietf.org/doc/html/rfc6901)) to fetch a token from 
the JSON tree. A token has an `.address` property that 
points to the string associated with the token. It also
has a `.kind` property.

We used our own pool of memory instead of using the 
shared static one, since our object will point to it, and we don't
want to lose the value.