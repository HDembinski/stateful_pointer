# stateful_pointer

Sometimes space is tight. On a 64-bit system, a pointer occupies 64 bit, but not all bits are needed to address aligned memory. This C++11 library provides pointer types which allow you to use up to 24 bits of the 64 bits to store arbitrary extra state. This works on all platforms supported by [Boost.Align](http://www.boost.org/doc/libs/1_65_1/doc/html/align.html).

Code is released under the Boost License v1.0 (see LICENCE file).
