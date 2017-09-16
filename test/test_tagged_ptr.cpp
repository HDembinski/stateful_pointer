#include "boost/stateful_pointer/tagged_ptr.hpp"
#include "boost/core/lightweight_test.hpp"
#include "boost/utility/binary.hpp"

int main() {
    using namespace boost::stateful_pointer;

    {
        BOOST_TEST_EQ(detail::make_ptr_mask(0), ~0);
        BOOST_TEST_EQ(detail::make_ptr_mask(1), ~1);
        BOOST_TEST_EQ(detail::make_ptr_mask(2), ~(1 | 2));
        BOOST_TEST_EQ(detail::make_ptr_mask(3), ~(1 | 2 | 4));
    }

    static unsigned destructor_count_test_type = 0;

    struct test_type {
        int a; char b;
        test_type() : a(0), b(0) {}
        test_type(int x, char y) : a(x), b(y) {}
        ~test_type() { ++destructor_count_test_type; }
    };

    // check that tagged_ptr has the same size as void*
    BOOST_TEST_EQ(sizeof(tagged_ptr<test_type, 2>), sizeof(void*));

    destructor_count_test_type = 0;
    {   // basic usage
        auto p = make_tagged_ptr<test_type, 2>(2, 3);

        BOOST_TEST(!!p);
        BOOST_TEST_EQ(static_cast<bool>(p), true);
        BOOST_TEST_EQ(p.bits(), BOOST_BINARY( 00 ));
        BOOST_TEST_EQ(p.bit(0), false);
        BOOST_TEST_EQ(p.bit(1), false);
        BOOST_TEST_EQ((*p).a, 2);
        BOOST_TEST_EQ(p->b, 3);

        p.bits(BOOST_BINARY( 01 ));
        BOOST_TEST_EQ(p.bits(), BOOST_BINARY( 01 ));
        BOOST_TEST_EQ(p->a, 2);
        BOOST_TEST_EQ(p->b, 3);
        p.bits(BOOST_BINARY( 10 ));
        BOOST_TEST_EQ(p.bits(), BOOST_BINARY( 10 ));
        BOOST_TEST_EQ(p->a, 2);
        BOOST_TEST_EQ(p->b, 3);

        p.bit(0, false);
        p.bit(1, true);
        BOOST_TEST_EQ(p.bit(0), false);
        BOOST_TEST_EQ(p.bit(1), true);
        BOOST_TEST_EQ(p->a, 2);
        BOOST_TEST_EQ(p->b, 3);
        p.bit(0, true);
        p.bit(1, false);
        BOOST_TEST_EQ(p.bit(0), true);
        BOOST_TEST_EQ(p.bit(1), false);
        BOOST_TEST_EQ(p->a, 2);
        BOOST_TEST_EQ(p->b, 3);

        p.reset();
        BOOST_TEST(p == (tagged_ptr<test_type, 2>()));
    }
    BOOST_TEST_EQ(destructor_count_test_type, 1);

    destructor_count_test_type = 0;
    {   // null vs default constructed
        auto p = tagged_ptr<test_type, 3>(); // null
        BOOST_TEST(p == (tagged_ptr<test_type, 3>()));
        p.bits(BOOST_BINARY( 101 ));
        BOOST_TEST(!p); // still nullptr...
        BOOST_TEST(p != (tagged_ptr<test_type, 3>())); // ... but not null

        auto q = make_tagged_ptr<test_type, 3>(); // default constructed
        BOOST_TEST(q != (tagged_ptr<test_type, 3>())); // not nullptr
        q.bits(BOOST_BINARY( 101 ));
        BOOST_TEST_EQ(p.bits(), q.bits());
    }
    BOOST_TEST_EQ(destructor_count_test_type, 1);

    {   // release
        destructor_count_test_type = 0;
        test_type* tp = nullptr;
        {
            auto p = make_tagged_ptr<test_type, 2>(2, 3);
            tp = p.release();
        }
        BOOST_TEST_EQ(destructor_count_test_type, 0);
        delete tp;
        BOOST_TEST_EQ(destructor_count_test_type, 1);
    }

    destructor_count_test_type = 0;
    {
        auto p = make_tagged_ptr<test_type, 2>(2, 3);
        p.bit(0, false);
        p.bit(1, true);

        // move ctor
        tagged_ptr<test_type, 2> q(std::move(p));
        BOOST_TEST_EQ(q.bit(0), false);
        BOOST_TEST_EQ(q.bit(1), true);
        BOOST_TEST_EQ(q->a, 2);
        BOOST_TEST_EQ(q->b, 3);

        // move assign
        tagged_ptr<test_type, 2> r;
        r = std::move(q);
        BOOST_TEST_EQ(r.bit(0), false);
        BOOST_TEST_EQ(r.bit(1), true);
        BOOST_TEST_EQ(r->a, 2);
        BOOST_TEST_EQ(r->b, 3);
    }
    BOOST_TEST_EQ(destructor_count_test_type, 1);

    destructor_count_test_type = 0;
    {   // swap
        auto p = make_tagged_ptr<test_type, 3>(2, 3);
        p.bits(BOOST_BINARY( 101 ));
        auto q = make_tagged_ptr<test_type, 3>(4, 5);
        q.bits(BOOST_BINARY( 010 ));

        std::swap(p, q);

        BOOST_TEST_EQ(p.bits(), BOOST_BINARY( 010 ));
        BOOST_TEST_EQ(p->a, 4);
        BOOST_TEST_EQ(p->b, 5);
        BOOST_TEST_EQ(q.bits(), BOOST_BINARY( 101 ));
        BOOST_TEST_EQ(q->a, 2);
        BOOST_TEST_EQ(q->b, 3);

        p.swap(q);

        BOOST_TEST_EQ(p.bits(), BOOST_BINARY( 101 ));
        BOOST_TEST_EQ(p->a, 2);
        BOOST_TEST_EQ(p->b, 3);
        BOOST_TEST_EQ(q.bits(), BOOST_BINARY( 010 ));
        BOOST_TEST_EQ(q->a, 4);
        BOOST_TEST_EQ(q->b, 5);
    }
    BOOST_TEST_EQ(destructor_count_test_type, 2);

    static unsigned destructor_count_base = 0;
    struct base {
        int a = 1;
        virtual ~base() { ++destructor_count_base; }
    };

    static unsigned destructor_count_derived = 0;
    struct derived : public base {
        char b = 2;
        virtual ~derived() { ++destructor_count_derived; }
    };

    struct derived2 : public base {};

    {   // conversion derived <-> base
        auto d = make_tagged_ptr<derived, 3>();
        d.bits(BOOST_BINARY( 101 ));
        tagged_ptr<base, 3> b = std::move(d);
        BOOST_TEST_EQ(b.bits(), BOOST_BINARY( 101 ));
        BOOST_TEST_EQ(b->a, 1);
        auto dp = dynamic_cast<derived*>(b.get());
        BOOST_TEST(dp);
        BOOST_TEST_EQ(dp->a, 1);
        BOOST_TEST_EQ(dp->b, 2);
        auto d2p = dynamic_cast<derived2*>(b.get());
        BOOST_TEST(!d2p);
    }
    BOOST_TEST_EQ(destructor_count_base, 1);
    BOOST_TEST_EQ(destructor_count_derived, 1);

    return boost::report_errors();
}
