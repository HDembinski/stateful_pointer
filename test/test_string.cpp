#include "stateful_pointer/string.hpp"
#include "boost/core/lightweight_test.hpp"
#include "stdexcept"
#include "algorithm"
#include "sstream"

using namespace stateful_pointer;

int main() {

    {   // ctors
        string s1;
        BOOST_TEST_EQ(s1.size(), 0u);
        BOOST_TEST(s1.empty());

        BOOST_TEST_THROWS(string(nullptr), std::logic_error);
        
        string s2("");
        BOOST_TEST(s2.empty());
        BOOST_TEST_EQ(s2.size(), 0);

        string s3("abc"); // uses small string optimisation
        BOOST_TEST(!s3.empty());
        BOOST_TEST_EQ(s3.size(), 3);
        BOOST_TEST_EQ(std::distance(s3.begin(), s3.end()), 3u);
        BOOST_TEST_EQ(s3[0], 'a');
        BOOST_TEST_EQ(s3[1], 'b');
        BOOST_TEST_EQ(s3[2], 'c');
        BOOST_TEST(s3 == "abc");

        string s3a("abcdefghijklmnopqrstuvwxyz"); // uses normal allocation
        BOOST_TEST(!s3a.empty());
        BOOST_TEST_EQ(s3a.size(), 26);
        BOOST_TEST(s3a == "abcdefghijklmnopqrstuvwxyz");

        string s4(s3a, 24);
        BOOST_TEST(!s4.empty());
        BOOST_TEST_EQ(s4.size(), 2);
        BOOST_TEST(s4 == "yz");

        string s5(s3a, 1, 20);
        BOOST_TEST(!s5.empty());
        BOOST_TEST_EQ(s5.size(), 20);
        BOOST_TEST(s5 == "bcdefghijklmnopqrstu");

        string s6(5, 'a');
        BOOST_TEST(!s6.empty());
        BOOST_TEST_EQ(s6.size(), 5);
        BOOST_TEST(s6 == "aaaaa");
    }

    {   // ostream operator
        std::ostringstream os1;
        string s1("abc");
        os1 << s1;
        BOOST_TEST_EQ(s1, os1.str());

        std::ostringstream os2;
        string s2("abcdefghijklmnopqrstuvwxyz");
        os2 << s2;
        BOOST_TEST_EQ(s2, os2.str());
    }

    return boost::report_errors();
}
