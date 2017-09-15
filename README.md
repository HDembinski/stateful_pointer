# Stateful Pointer

Sometimes space is tight. On modern systems, a pointer occupies 64 bit, but not all bits are needed to address aligned memory. This C++11 library provides pointer types which allow you to use up to 24 bits of the 64 bits to store arbitrary extra state. The mechanism works on all platforms, and on many platforms with zero overhead. supported by [Boost.Align](http://www.boost.org/doc/libs/1_65_1/doc/html/align.html).

Code is released under the Boost License v1.0 (see LICENSE file).
