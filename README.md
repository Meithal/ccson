CCSON is a [CSON](http://noe.mearie.org/cson/) library in C. 
It comes as a single file and has one function that converts
CSON text to JSON text.

## Requirements
* C11 compiler, with `stdtypes.h` and `uchar.h` 
standard headers. Tested with MSVC and GCC.

## Example

```c
#define CSON_IMPLEMENTATION
#include "cson.h"

int main(void) {
    unsigned char sink[1000] = { 0 };
    puts(cson_to_json("pi: 3.141592, e = 2.718281828, 'foo': 'bar'", sink, 1000));
    /* 
     * {
          "pi": 3.141592,
          "e": 2.718281828,
          "foo": "bar"
        } 
    */
}
```

Many more examples and use cases are in the tests file.