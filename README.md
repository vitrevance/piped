# Piped

Piped is a single header library for C++ that features piping operator syntax similar to that of Circle compiler, but in pure C++ 20 standard, without any compiler extensions.

## Installation
Simply include the piped.hpp header file in your C++ project to start using the library.

## Usage
```c++
#include "piped.hpp"
#include <iostream>

using piped::$;

int main() {
    auto result = $[5] || (!$ * 2) || (!$ + 3) || $; // Pipe into $ at the end to extract result
    $[5] || (!$ * 2) || (!$ + 3); // Ignore result
    std::cout << result << std::endl; // Output: 13
    return 0;
}
```

For more examples, see tests.

## Reference
- For more information on the Circle compiler, visit [Circle Lang](https://www.circle-lang.org/).
- See [cache consistent version](https://github.com/vitrevance/piped/tree/consistent) of Piped
