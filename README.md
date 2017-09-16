# Stateful Pointer Library

Sometimes space is tight! What if you could squeeze extra state into a pointer to an arbitrary object?

On modern systems, a pointer occupies 64 bit, but not all bits are used because of memory alignment. The *Stateful Pointer Library* is a C++11 header-only library. It provides pointer types analog to `std::unique_ptr` that allow you to use up to 24 bits of the 64 bits to store extra state inside the pointer. These freely useable bits occupy *no extra space*.

The library uses [Boost.Align](http://www.boost.org/doc/libs/1_65_1/doc/html/align.html) to allocate aligned memory on all platforms. On most platforms (Windows, MacOS, Linux, Android, ...) this incurs at no overhead. On unsupported platforms, extra memory is allocated to guarantee the alignment of the pointer. In either case, the pointers of the *Stateful Pointer Library* are guaranteed to have the same size as a normal pointer.

In optimized builds, creation and access performance of the special pointers is similar to `std::unique_ptr`, access is as fast and creation is at most the 10 % slower.

Code is released under the Boost License v1.0 (see LICENSE file).

## Tagged pointer

Like `std::unique_ptr` in interface and behavior, but encodes `N` extra bits of information inside pointer, using no additional space.

```C++
#include <stateful_pointer/tagged_ptr.hpp>
#include <boost/utility/binary.hpp> // macro to make binary literals

// tagged_ptr has the same size as a normal pointer
static_assert(sizeof(tagged_ptr<A, 4>) == sizeof(void*));

int main() {
    using namespace stateful_pointer;

    // class to be allocated on the heap
    struct A {
        int a;
        A(int x) : a(x) {}
    };

    // make tagged_ptr to an instance of A with 4 bits of extra state
    auto p = make_tagged_ptr<A, 4>(3); // 3 is passed to the ctor of A

    // set the 4 bits to some values
    p.bits(BOOST_BINARY( 1010 )); // that's 10 in decimal

    std::cout << "a = " << p->a << ", bits = " << p.bits() << std::endl;

    // prints: "a = 3, bits = 10"
}
```
