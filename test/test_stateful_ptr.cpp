#include "boost/stateful_pointer/stateful_ptr.hpp"
#include "boost/core/lightweight_test.hpp"

int main() {
    using namespace boost::stateful_pointer;

    {
        BOOST_TEST_EQ(detail::ptr_mask(0), ~0);
        BOOST_TEST_EQ(detail::ptr_mask(1), ~1);
        BOOST_TEST_EQ(detail::ptr_mask(2), ~(1 | 2));        
    }

    struct s { int a; char b; };

    // stateful_ptr has the same size as void*
    BOOST_TEST_EQ(sizeof(stateful_ptr<s, 2>), sizeof(void*));

    {   // basic usage
        stateful_ptr<s, 2> p(s{2, 3});

        BOOST_TEST_EQ(p.bit(0), false);
        BOOST_TEST_EQ(p.bit(1), false);
        BOOST_TEST_EQ((*p).a, 2);
        BOOST_TEST_EQ(p->b, 3);

        p.bit(0, true);
        p.bit(1, false);
        BOOST_TEST_EQ(p.bit(0), true);
        BOOST_TEST_EQ(p.bit(1), false);
        BOOST_TEST_EQ(p->a, 2);
        BOOST_TEST_EQ(p->b, 3);

        p.bit(0, false);
        p.bit(1, true);
        BOOST_TEST_EQ(p.bit(0), false);
        BOOST_TEST_EQ(p.bit(1), true);
        BOOST_TEST_EQ(p->a, 2);
        BOOST_TEST_EQ(p->b, 3);
    }

    {   // copy
        stateful_ptr<s, 2> p(s{2, 3});
        p.bit(0, false);
        p.bit(1, true);
        stateful_ptr<s, 2> q(p);
        BOOST_TEST_EQ(q.bit(0), false);
        BOOST_TEST_EQ(q.bit(1), true);
        BOOST_TEST_EQ(q->a, 2);
        BOOST_TEST_EQ(q->b, 3);

        // copy assign
        stateful_ptr<s, 2> r;
        r = p;
        BOOST_TEST_EQ(r.bit(0), false);
        BOOST_TEST_EQ(r.bit(1), true);
        BOOST_TEST_EQ(r->a, 2);
        BOOST_TEST_EQ(r->b, 3);
    }

    {   // move
        stateful_ptr<s, 2> p(s{2, 3});
        p.bit(0, false);
        p.bit(1, true);
        stateful_ptr<s, 2> q(std::move(p));
        BOOST_TEST_EQ(q.bit(0), false);
        BOOST_TEST_EQ(q.bit(1), true);
        BOOST_TEST_EQ(q->a, 2);
        BOOST_TEST_EQ(q->b, 3);

        // move assign
        stateful_ptr<s, 2> r;
        r = std::move(q);
        BOOST_TEST_EQ(r.bit(0), false);
        BOOST_TEST_EQ(r.bit(1), true);
        BOOST_TEST_EQ(r->a, 2);
        BOOST_TEST_EQ(r->b, 3);
    }

    return boost::report_errors();
}
