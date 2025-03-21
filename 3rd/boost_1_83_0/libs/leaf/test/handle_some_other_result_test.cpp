// Copyright 2018-2022 Emil Dotchevski and Reverge Studios, Inc.

// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/leaf/config.hpp>

#if !BOOST_LEAF_CFG_STD_SYSTEM_ERROR

#include <iostream>

int main()
{
    std::cout << "Unit test not applicable." << std::endl;
    return 0;
}

#else

#ifdef BOOST_LEAF_TEST_SINGLE_HEADER
#   include "leaf.hpp"
#else
#   include <boost/leaf/handle_errors.hpp>
#   include <boost/leaf/pred.hpp>
#endif

#include "_test_res.hpp"
#include "lightweight_test.hpp"

namespace leaf = boost::leaf;

template <int> struct info { int value; };

template <class ResType>
ResType f( bool succeed )
{
    if( succeed )
        return 42;
    else
        return make_error_code(errc_a::a0);
}

template <class ResType>
ResType g( bool succeed )
{
    if( auto r = f<ResType>(succeed) )
        return r;
    else
        return leaf::error_id(r.error()).load(info<42>{42}).to_error_code();
}

template <class ResType>
void test()
{
    {
        ResType r = leaf::try_handle_some(
            []
            {
                return g<ResType>(true);
            } );
        BOOST_TEST(r);
        BOOST_TEST_EQ(r.value(), 42);
    }
    {
        int called = 0;
        ResType r = leaf::try_handle_some(
            [&]
            {
                auto r1 = g<ResType>(false);
                BOOST_TEST(!r1);
                auto ec = r1.error();
                BOOST_TEST_EQ(ec.message(), "LEAF error");
                BOOST_TEST(!std::strcmp(ec.category().name(),"LEAF error"));
                return r1;
            },
            [&]( info<42> const & x, leaf::match<leaf::condition<cond_x>, cond_x::x00> ec )
            {
                called = 1;
                BOOST_TEST_EQ(x.value, 42);
                return ec.matched;
            } );
        BOOST_TEST(!r);
        BOOST_TEST_EQ(r.error(), make_error_code(errc_a::a0));
        BOOST_TEST(called);
    }
}

int main()
{
    test<test_res<int, std::error_code>>();
    test<test_res<int const, std::error_code>>();
    test<test_res<int, std::error_code> const>();
    test<test_res<int const, std::error_code> const>();
    return boost::report_errors();
}

#endif
