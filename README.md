# Stateful Pointer Library

Sometimes space is tight! What if you could squeeze *extra state* into a pointer *at (almost) no additional cost*?

A pointer occupies 32 or 64 bit, which is 4 or 8 bytes just to remember a memory address. Not all of those bits are actually used because the computer needs *aligned* memory addresses. The *Stateful Pointer Library* is a platform-independent C++11 header-only library. It provides a smart pointer which mimics `std::unique_ptr`, but allows you to use up to 24 bits to store extra state inside the pointer in a safe and platform-independent way. These freely useable bits are encoded inside the pointer itself and occupy *no extra space*.

The library uses [Boost.Align](http://www.boost.org/doc/libs/1_65_1/doc/html/align.html) to allocate aligned memory on all platforms. On most platforms (Windows, MacOS, Linux, Android, ...), special system calls are used to get aligned memory at no additional cost. On other platforms, extra memory is allocated to guarantee the alignment of the pointer. The amount grows with the number of bits in the pointer that are used to carry extra state. In either case, the pointers of the *Stateful Pointer Library* are guaranteed to have the same size as a normal pointer.

**Caveat**: The library uses custom memory allocation to work its magic, so it does *not* work with classes/environments that also customize heap allocation.

Code is released under the **Boost Software License v1.0** (see LICENSE file).

## Tagged pointer

Like `std::unique_ptr` in interface and behavior, but encodes `N` extra bits of information inside the pointer, using no additional space.

```c++
#include "stateful_pointer/tagged_ptr.hpp"
#include "boost/utility/binary.hpp" // macro to make binary literals
#include <cassert>

using namespace stateful_pointer;

int main() {

    // class to be allocated on the heap
    struct A {
        int a;
        A(int x) : a(x) {}
    };

    // tagged_ptr has the same size as a normal pointer
    assert(sizeof(tagged_ptr<A, 4>) == sizeof(void*));

    // make tagged_ptr to an instance of A with 4 bits of extra state
    auto p = make_tagged<A, 4>(3); // 3 is passed to the ctor of A

    // set the 4 bits to some values
    p.bits(BOOST_BINARY( 1010 )); // that's 10 in decimal

    std::cout << "a = " << p->a << ", bits = " << p.bits() << std::endl;

    // prints: "a = 3, bits = 10"
}
```

## String

The World's most compact STL-compliant string with *small string optimization*. Has the size of a mere pointer and yet stores up to 7 characters (on a 64-bit system) without allocating extra memory on the heap.

```c++
#include "stateful_pointer/string.hpp"
#include <cassert>

using namespace stateful_pointer;

int main() {
    // string has the same size as a normal pointer
    assert(sizeof(string) == sizeof(void*));

    string s("foo bar"); // small string optimisation: no heap allocation
    std::cout << s << std::endl;

    // prints: "foo bar"
}
```

This one is still in development, a lot of the standard interface is still missing.

## Performance

### `tagged_ptr` vs `std::unique_ptr`

In optimized builds, the performance is similar. Most importantly, access is as fast. Pointer creation is at most 10 % slower and becomes negligible compared to the allocation and initialization cost of larger pointees.

|Benchmark                                   |CPU [ns]|
|:-------------------------------------------|-------:|
|`unique_ptr_creation<char>`                 |      29|
|`tagged_ptr_creation<char>`                 |      32|
|`unique_ptr_creation<std::array<char, 256>>`|      70|
|`tagged_ptr_creation<std::array<char, 256>>`|      72|
|`unique_ptr_access<char>`                   |       2|
|`tagged_ptr_access<char>`                   |       2|
|`unique_ptr_access<std::array<char, 256>>`  |       2|
|`tagged_ptr_access<std::array<char, 256>>`  |       2|

([Google benchmark library](https://github.com/google/benchmark) run on 4x3GHz CPUs, compiled with -O3)
