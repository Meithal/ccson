Cisson is a small JSON library in C that focuses on being
non-disruptive. It will not tamper with your data.

It doesn't need a stdlib but can use it
if it gives you better performance.

It compiles by default in ANSI C but optionally
can compile and use recent features of C11 for
better performance.

## Install
The most easy way to use this library is the header-only file
`json_single_header.h`. You include it or copy/paste its contents
in the file you need the JSON feature. 

For use as a library you must have CMake installed. The
default CmakeList script will generate a static and dynamic
library.
You have to use `target_link_libraries()` to use any of them.
`sjson` is the static library target, 
`xjson` is the dynamic library target.

To bake cisson into your program, you can also list 
`json.h` and `njson.c` in your `add_executable` command.

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
If we have a C object like this
```c
struct foo = {
        .text = "text",
        .array = {1, 2, 3}
};
```
we can turn it into JSON text like that
```c
struct state state = {0};
memset(static_stack, 0, sizeof(static_stack));
memset(static_pool, 0, sizeof(static_pool));
state.tokens.tokens_stack = static_stack;
state.copies.string_pool = static_pool;

START_AND_PUSH_TOKEN(&state, ROOT, "#custom root");
START_AND_PUSH_TOKEN(&state, OBJECT, "{");
PUSH_ROOT(&state);
START_AND_PUSH_TOKEN(&state, STRING, "\"text\"");
PUSH_ROOT(&state);
START_AND_PUSH_TOKEN(&state, STRING, "\"text\"");
CLOSE_ROOT(&state);
CLOSE_ROOT(&state);
START_AND_PUSH_TOKEN(&state, ARRAY, "[");
PUSH_ROOT(&state);
START_AND_PUSH_TOKEN(&state, NUMBER, "1");
START_AND_PUSH_TOKEN(&state, NUMBER, "2");
START_AND_PUSH_TOKEN(&state, NUMBER, "3");
puts(to_string(&state.tokens));
```
Note that closing tokens are not necessary.
