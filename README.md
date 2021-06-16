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

For use as a library you must have CMake installed. The
default CmakeList script will generate a static and dynamic
library.
You have to use `target_link_libraries()` to use any of them.
`sjson` is the static library target, 
`xjson` is the dynamic library target.

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
        .array = {1, 2, 3}
};
```
we can turn it into JSON text like that
```c
#define CISSON_IMPLEMENTATION
#include "cisson.h"

struct cisson_state cisson_state = {0};

memset(static_stack, 0, sizeof(static_stack));
memset(static_pool, 0, sizeof(static_pool));

cisson_state.tokens.tokens_stack = static_stack;
cisson_state.copies.string_pool = static_pool;

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
START_AND_PUSH_TOKEN(&cisson_state, ARRAY, "[");
PUSH_ROOT(&cisson_state);
START_AND_PUSH_TOKEN(&cisson_state, NUMBER, "1");
START_AND_PUSH_TOKEN(&cisson_state, NUMBER, "2");
START_AND_PUSH_TOKEN(&cisson_state, NUMBER, "3");
puts(to_string(&cisson_state.tokens));
```
Note that closing tokens are not necessary. More and
up-to-date examples are in tests/tests.c.

