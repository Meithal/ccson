Cisson is a JSON library in C. It parses JSON into an abstract
tree and serializes trees into JSON. It lets you manipulate 
trees in various ways.

## Requirements
Tested with GCC and MSVC.

## Install
Used single-header
```c
#define CISSON_IMPLEMENTATION
#include "cisson.h"
```
The preprocessor will copy `cisson.h` in your file, and
there is nothing more to do on your side. You must define
`CISSON_IMPLEMENTATION` only once in all your code base.

## Usage
`rjson()` reads JSON and turns it into a cisson object
(a tree of tokens). `inject()` reads raw JSON and
injects it into an existing tree.

`to_string()` turns a cisson tree object into JSON text.
`to_string_pointer()` extracts a subpart of the tree and turns
it into JSON text.

`push_token()`, `insert_token()`, `stream_tokens()` and
`stream_into()` let you build a cisson tree from scratch, 
or manipulate one that already exists. 

`query()` lets you target a specific token in the tree using
the JSON pointer syntax.

`move()` and `delete()` let you manipulate an existing tree.

## Examples

```c
#define CISSON_IMPLEMENTATION
#include "cisson.h"
#include <stdio.h>

int main() {
    struct cisson_state state = {0};
    rjson("{\"mon\":[],\"tue\":[]}", &state);

    struct token* mon = query(&state, "/mon");
    struct token* tue = query(&state, "/tue");
    char buf[4];
    int j;
    for (j = 0; j < 20; j++) {
        sprintf(buf, "%d", j);
        insert_token(&state, buf, j % 2 ? mon : tue);
    }
    
    puts(to_string(&state.tokens));
    assert(strcmp(
            to_string_compact(&state.tokens), 
            "{\"mon\":[1,3,5,7,9,11,13,15,17,19],\"tue\":[0,2,4,6,8,10,12,14,16,18]}"
            ) == 0);
    }
```

`rjson` reads raw JSON and converts it to a cisson tree object.

`inject` reads raw JSON, and injects it in an existing tree, at
the place pointed by the third argument.

`query` uses a JSON pointer
(see [RFC 6901](https://datatracker.ietf.org/doc/html/rfc6901))
to fetch a token from the JSON tree. It is extended to allow
targeting keys themselves.

`to_string` and `to_string_compact` convert a cisson tree into
JSON text.


Cisson only accepts string values, you must convert your non-string
values before they can be tokenized; by using `sprintf` for example.

`to_string` converts your cisson object into a JSON string
and returns a pointer to it.

```c
#define CISSON_IMPLEMENTATION
#include "cisson.h"
#include <stdio.h>

int main() {
    struct cisson_state state = {0};
    
    /* cisson has a static stack of tokens and a static pool
     * of characters where the token contents will be copied to.
     * You can use your own stack and pool if you plan to 
     * have more than one cisson object in memory at any time. */

    struct cisson_state state = { 0 };
    struct cisson_state state2 = { 0 };
    
    struct token stack1[40];
    struct token stack2[40];
    unsigned char pool1[400];
    unsigned char pool2[400];
    
    start_state(&state, stack1, sizeof stack1, pool1, sizeof pool1);
    start_state(&state2, stack2, sizeof stack2, pool2, sizeof pool2);

    rjson("[1, 2, 3, [4, 5, 6], [7, 8, 9]", &state);
    rjson("{\"foo\": 1}", &state2);
    
    insert_token(&state2, "\"array\"", query(&state2, ""));
    move(&state, query(&state, "/1"), query(&state, "/4"));
    inject(to_string_pointer( &state, query(&state, "/3")), &state2,
        query(&state2, "/array/<"));
    delete(query(&state, "/3"));
    rename_string(&state2, query(&state2, "/foo/<"), "bar");
    
    puts(to_string_compact(&state.tokens)); /* [1,3,[4,5,6]] */
    puts(to_string_compact(&state2.tokens)); /* {"bar":1,"array":[7,8,9,2]} */

}
```
We create two different states. Since two states
can't share the same pool, we create our own 
custom pools and stacks and signal our states that they have
to use them with `start_state`. 

`start_state()` let you customize the memory your cisson tree
will work with and print to, by default every json tree
uses the same static memory.

`insert_token` inserts a string at the topmost value (the
object) so its state is now `{"foo": 1, "array"}`. Note
that `push_token` and `insert_token` don't check for
grammar validity and, the state has now a JSON 
syntax error. On the other hand, `rjson` and `inject` can only
work with valid JSON.

`move` moves a token to another root. First state is now
`[1, 3, [4, 5, 6], [7, 8, 9, 2]`.

We cannot use `move` between two different tree states,
so we extract the JSON verbatim with `to_string_pointer`
from one tree and `inject` it to the other tree. The second 
tree state is now `{"foo":1,"array":[7,8,9,2]}`. Note that
our JSON pointer has a terminating `<`: that means we want 
to target the string token itself, and not the value it leads to
(which doesn't exist at this point).

We then `delete` the array we moved from the first state,
so its state is now `[1,3,[4,5,6]]`.

We then `rename_string` the string `"foo"` to `"bar"` in the second
state. This way is more efficient than creating a new string,
delete the previous one and, make its children point on the new
string and inject it inplace of the ancient one.

## Caveats
All the tokens have their string representation copied to a pool
of characters. Every string copied that way are contiguous one next
to another. You shouldn't try to modify a string once it has been 
copied to the poll. It is better to create a new string and make the tree
token point to it.

Values of a tree that are expected to often change should
be extracted to a native type (int, float...), then injected
in the tree once you actually need there an up-to-date value.

Extracting more tokens than the token stack can hold
has undefined behavior. Any value will generate a token, those
  are `null`, `true`, `false`, a `number`, a `string`.
  Compound values, `arrays` and `objects`, only generate one token,
  themselves. The comma `,` separator characters, the colon `:`, and
  the closing characters `}`, `]`; don't generate a token.

Extracting more text content than the string pool can hold 
  has undefined behavior. For consistency, any token will 
  hold the string it points to. A true pointer will point
  to `true`, a string to `"string"`, an object to a single 
`{` character, and so on.
  An array of ten `true` tokens will make the pool 
  store ten `true` strings and a `[` string, plus their
respective size, stored by default in an 
`unsigned char`.

### Thread safety
The library is not thread safe but can be if you provide
yourself the memory your trees work with.

### Encoding
Only UTF-8 (with optional BOM) is supported. Encoding errors
propagate through as-is. Control characters are \u escaped.

### Binary safe strings
Most documented functions are actually macros that compute
the length of your strings through `strlen` before calling
the actual function you are looking for. 
Those functions have an underscore suffix.
If you need to handle binary safe strings, use those 
functions directly.

### Nesting
Does not involve recursion, can parse nested objects as deep 
as INT_MAX.

### Duplicate keys, ordering
Object keys can have duplicate names. 

The tree don't keep in memory the order of array elements or
object keys:
they are printed mostly in the order they have been 
inserted on the token stack. There is no way to specify
where you want to insert an element inside an array.

## Preprocessor options
`WANT_LIBC` includes the standard C library instead of
using drop-in replacement functions. 

`WANT_JSON1` gives  the option to parse a JSON string
in the first standard way, that allowed only arrays and 
objects as topmost values.

`SELF_MANAGE_MEMORY` prevents the library to allocate static
memory to use as default token stack and string pool. If
you don't define this, you can then define `STRING_POOL_SIZE`
and `MAX_TOKENS` to define their respective sizes.

`FORCE_ANSI` fores the code to look like ANSI C, even
if your compiler can support more recent versions of C.