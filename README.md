# Stateful Pointer Library

Sometimes space is tight! What if you could squeeze extra state into a pointer to an arbitrary object?

On modern systems, a pointer occupies 64 bit, but not all bits are used because of memory alignment. The C++11 *Stateful Pointer Library* provides pointer types which allow you to use up to 24 bits of the 64 bits to store extra state inside the pointer. The library uses [Boost.Align](http://www.boost.org/doc/libs/1_65_1/doc/html/align.html) to obtain pointers to aligned memory, which works on all platforms and on many with zero overhead.

Code is released under the Boost License v1.0 (see LICENSE file).

This library is not part of Boost, but will be proposed for inclusion and therefore uses the `boost` namespace.

## Tagged pointer

Like `std::unique_ptr`, but stores `N` extra bits of information in the pointer.

```C++
#include <boost/stateful_pointer/tagged_ptr.hpp>
#include <boost/utility/binary.hpp> // macro to make binary literals

int main() {
    using namespace boost::stateful_pointer;

    // class to be allocated on the heap
    struct A {
        int a;
        A(int x) : a(x) {}
    };

    // make tagged_ptr to an instance of A with 4 bits of extra state
    auto p = make_tagged_ptr<A, 4>(3); // 3 is passed to the ctor of A

    // set the 4 bits to some values
    p.bits(BOOST_BINARY( 1010 )); // that's 20 in decimal

    std::cout << "a = " << p->a << ", bits = " << p.bits() << std::endl;

    // prints: "a = 3, bits = 20"
}
```
