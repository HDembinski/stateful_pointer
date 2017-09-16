#include "boost/stateful_pointer/tagged_ptr.hpp"
#include "boost/core/lightweight_test.hpp"
#include "boost/utility/binary.hpp"

static unsigned destructor_count = 0;

int main() {
    using namespace boost::stateful_pointer;

    {
        BOOST_TEST_EQ(detail::make_ptr_mask(0), ~0);
        BOOST_TEST_EQ(detail::make_ptr_mask(1), ~1);
        BOOST_TEST_EQ(detail::make_ptr_mask(2), ~(1 | 2));
        BOOST_TEST_EQ(detail::make_ptr_mask(3), ~(1 | 2 | 4));
    }

    struct test_type {
        int a; char b;
        test_type() : a(0), b(0) {}
        test_type(int x, char y) : a(x), b(y) {}
        ~test_type() { ++destructor_count; }
    };

    // check that tagged_ptr has the same size as void*
    BOOST_TEST_EQ(sizeof(tagged_ptr<test_type, 2>), sizeof(void*));

    destructor_count = 0;
    {   // basic usage
        auto p = make_tagged_ptr<test_type, 2>(2, 3);

        BOOST_TEST_EQ(!p, false);
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
    BOOST_TEST_EQ(destructor_count, 1);

    destructor_count = 0;
    {   // null vs default constructed
        auto p = tagged_ptr<test_type, 3>(); // nullptr
        BOOST_TEST(p == (tagged_ptr<test_type, 3>()));
        p.bits(BOOST_BINARY( 101 ));
        BOOST_TEST_EQ(!p, true);
        BOOST_TEST(p != (tagged_ptr<test_type, 3>()));

        auto q = make_tagged_ptr<test_type, 3>(); // default constructed
        BOOST_TEST(q != (tagged_ptr<test_type, 3>()));
        q.bits(BOOST_BINARY( 101 ));
        BOOST_TEST_EQ(p.bits(), q.bits());
    }
    BOOST_TEST_EQ(destructor_count, 1);

    {   // release
        destructor_count = 0;
        test_type* tp = nullptr;
        {
            auto p = make_tagged_ptr<test_type, 2>(2, 3);
            tp = p.release();
        }
        BOOST_TEST_EQ(destructor_count, 0);
        delete tp;
        BOOST_TEST_EQ(destructor_count, 1);
    }

    destructor_count = 0;
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
    BOOST_TEST_EQ(destructor_count, 1);

    return boost::report_errors();
}
