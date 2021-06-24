Cisson is a JSON library in C. It serializes C objects
into JSON and parses JSON into C objects.

## Requirements
Tested with GCC and MSVC.

## Install
Used single-header
```c
#define CISSON_IMPLEMENTATION
#include "cisson.h"
```
The preprocessor will copy `cisson.h` in your file, and
there is nothing more to do on your side.

## Usage
`rjson()` reads JSON and turns it into a cisson object
(a tree of tokens).

`to_string()` turns a cisson tree object into JSON text.

`push_token()`, `stream_tokens()` and `insert_token()` 
let you build a cisson tree from scratch or manipulate
one that already exists. 

`query()` lets you target a specific token in the tree using
the JSON pointer syntax.

`move()` and `delete()` let you manipulate an existing tree.

## Example

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

    stream_into(&state, mon, '+', &(char[]) {"true+false"});
    inject("[1, 2, 3]", &state, query(&state, "/tue"));

    puts(to_string(&state.tokens));
    assert(strcmp(
            to_string_compact(&state.tokens), 
            "{\"mon\":[1,3,5,7,9,11,13,15,17,19,true,false],\"tue\":[0,2,4,6,8,10,12,14,16,18,[1,2,3]]}"
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
Every token has a root that it binds to. The state keeps
in memory what the current root is, for example if the current root
is an array, every token we push will be a value of this array.
This allows to resume the building of the JSON tree at any time.

Array elements have as root their array;
Object properties have as root their object;
The values associated to object properties have as root that
very property. 

When we meet any of these characters `{`, `[`, `:`, `#`
, we change the root the next tokens will have their root as. 
When meeting `>`, the current root will become what the root of the
current root was, hence going back in the
hierarchy of roots by one notch. The nature of the root
we close doesn't matter, so we use a generic character. 

Cisson only accepts string values, you must convert your non-string
values before they can be tokenized; by using `sprintf` for example.

`to_string` converts your cisson object into a JSON string
and returns a pointer to it.

`stream_tokens` can be used to compress the previous code even further.

```c
#include "json.h"

int main() {
    struct cisson_state state = {0};

    stream_tokens(&state, '~',
        &(char[]){"#smart root~{~\"foo\"~:~\"bar\"~>~\"array\"~:~[~1~2~4~>>~\"question\"~:~true"});
    puts(to_string_compact(&state.tokens)); /* {"foo":"bar","array":[1,2,4],"question":true} */

}
```

We provide a separator character to use, and a stream of
tokens separated by the separator. The stream must be writeable.

You can mix `stream_tokens`, `push_tokens` and macros to
build a JSON object.

`start_state()` let you customize the memory your cisson tree
will work with and print to, by default every json tree
uses the same static memory.
