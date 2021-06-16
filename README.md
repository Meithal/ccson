Cisson is a JSON library in C. It can serialize C objects
into JSON and parse JSON into C objects.

It doesn't need a stdlib but can use one
if it gives you better performance.

It compiles by default in ANSI C.
It can compile and use recent features of C11 for
better performance.

## Requirements
Tested on Windows and Linux, should compile with MSVC and GCC.
Cmake is helpful to easily include this library in your project,
but not required if you use the single-header version of it.
No dependency, including LibC.

## Install
The most easy way to use this library is to `#include
cisson.h`. You must `#define CISSON_IMPLEMENTATION` 
before including the library that way.

To use cisson as a library you must have CMake installed.
`sjson` is the static library target, 
`xjson` is the dynamic library target.
You can use `target_link_libraries()` to link any of them to 
your program.

To bake cisson into your program, you can also list 
`json.h` and `njson.c` in your `add_executable` command.

**Note**: If you use cisson through cmake, you must `#include json.h`
instead of `cisson.h`, and you don't have to 
`#define CISSON_IMPLEMENTATION` before including `json.h`.

This example shows how to use it as a static library.
```bash
git clone https://gitlab.com/Meithal/cisson.git

echo -e "add_subdirectory(cisson)\n"\
"add_executable(my_exe my_source.c)\n"\ # this is your project
"target_link_libraries(sjson my_exe)" > CmakeLists.txt

mkdir build
cd build
cmake ..
cmake --build . --config Release
./Release/my_exe[.exe]
```

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

struct cisson_state cisson_state = {0};

start_state(&state, static_stack, sizeof static_stack,
            static_pool, sizeof static_pool);

START_AND_PUSH_TOKEN(&cisson_state, ROOT, "#custom root"); 
/* every token we push will be bound to the current root */
START_AND_PUSH_TOKEN(&cisson_state, OBJECT, "{");
PUSH_ROOT(&cisson_state);                                  
/* "PUSH_ROOT" will change the current root to the previously
 * pushed token so every following
 * token will be bound to the object token */ 
START_AND_PUSH_TOKEN(&cisson_state, STRING, "\"foo\"");
PUSH_ROOT(&cisson_state);
/* next token will be bound to the key */ 
START_AND_PUSH_TOKEN(&cisson_state, STRING, "\"bar\"");
CLOSE_ROOT(&cisson_state);
/* now the root is no more the key but the object */
START_AND_PUSH_TOKEN(&cisson_state, STRING, "\"array\"");
PUSH_ROOT(&cisson_state);
START_AND_PUSH_TOKEN(&cisson_state, ARRAY, "[");
PUSH_ROOT(&cisson_state);
START_AND_PUSH_TOKEN(&cisson_state, NUMBER, "1");
START_AND_PUSH_TOKEN(&cisson_state, NUMBER, "2");
START_AND_PUSH_TOKEN(&cisson_state, NUMBER, "3");
CLOSE_ROOT(&cisson_state); /* the array */
CLOSE_ROOT(&cisson_state); /* the object property */
START_AND_PUSH_TOKEN(&cisson_state, STRING, "\"question\"");
PUSH_ROOT(&cisson_state);
START_AND_PUSH_TOKEN(&cisson_state, TRUE, "true");
puts(to_string(&cisson_state.tokens)); /* {"foo":"bar","array":[1,2,4],"question":true} */
```
Note that closing tokens are not necessary if we have
no more tokens to push. More and
up-to-date examples are in tests/tests.c.

We can notice that the nature of the JSON token we push onto the stack
can be deduced from its first character. For example `[` signals an object,
`"` signals a string, and so on.
So, as a convenience, you can use the `push_token` function
instead of the macros. `push_token` can guess the nature
of the json token you want to add, without having you to 
provide its nature explicitly, by looking at the first character
of the token. If we had to rewrite the previous
example using that function, it would look like the following

```c
#include "json.h"
int main() {
    struct cisson_state cisson_state = {0};
    
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
`push_token` can deduce that it has to push a root when it encounters
any of the `{`, `[`, `:` characters. It cannot however deduce when you 
want to close a root. Since closing a root only changes
an internal counter inside the state, it doesn't matter whether
you push `}` or `]`, therefore we use the `>` symbol
as an all-purpose root closer. 
Note that you can close as many
roots as you want in the token string. Note also that 
`push_token` will use more computer cycles than the previous 
method, so, if more convenient, you may find it slower.