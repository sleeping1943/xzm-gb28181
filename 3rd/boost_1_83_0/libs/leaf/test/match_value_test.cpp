// Copyright 2018-2022 Emil Dotchevski and Reverge Studios, Inc.

// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifdef BOOST_LEAF_TEST_SINGLE_HEADER
#   include "leaf.hpp"
#else
#   include <boost/leaf/handle_errors.hpp>
#   include <boost/leaf/pred.hpp>
#   include <boost/leaf/result.hpp>
#endif

#include "_test_ec.hpp"
#include "lightweight_test.hpp"
#include <exception>

namespace leaf = boost::leaf;

enum class my_error { e1=1, e2, e3 };

struct e_my_error { my_error value; };

#if BOOST_LEAF_CFG_STD_SYSTEM_ERROR
struct e_error_code { std::error_code value; };
#endif

struct my_exception: std::exception
{
    int value;
};

template <class M, class E>
bool test(E const & e )
{
    if( M::evaluate(e) )
    {
        M m{e};
        BOOST_TEST_EQ(&e, &m.matched);
        return true;
    }
    else
        return false;
}

int main()
{
    {
        e_my_error e = { my_error::e1 };

        BOOST_TEST(( test<leaf::match_value<e_my_error, my_error::e1>>(e) ));
        BOOST_TEST(( !test<leaf::match_value<e_my_error, my_error::e2>>(e) ));
        BOOST_TEST(( test<leaf::match_value<e_my_error, my_error::e2, my_error::e1>>(e) ));
    }

#if BOOST_LEAF_CFG_STD_SYSTEM_ERROR
    {
        e_error_code e = { errc_a::a0 };

        BOOST_TEST(( test<leaf::match_value<leaf::condition<e_error_code, cond_x>, cond_x::x00>>(e) ));
        BOOST_TEST(( !test<leaf::match_value<leaf::condition<e_error_code, cond_x>, cond_x::x11>>(e) ));
        BOOST_TEST(( test<leaf::match_value<leaf::condition<e_error_code, cond_x>, cond_x::x11, cond_x::x00>>(e) ));

#if __cplusplus >= 201703L
        BOOST_TEST(( test<leaf::match_value<e_error_code, errc_a::a0>>(e) ));
        BOOST_TEST(( !test<leaf::match_value<e_error_code, errc_a::a2>>(e) ));
        BOOST_TEST(( test<leaf::match_value<e_error_code, errc_a::a2, errc_a::a0>>(e) ));
#endif
    }
#endif

    {
        int r = leaf::try_handle_all(
            []() -> leaf::result<int>
            {
                return leaf::new_error(e_my_error{my_error::e1});
            },

            []( leaf::match_value<e_my_error, my_error::e1> )
            {
                return 1;
            },

            []
            {
                return 2;
            } );
        BOOST_TEST_EQ(r, 1);
    }

    {
        int r = leaf::try_handle_all(
            []() -> leaf::result<int>
            {
                return leaf::new_error(e_my_error{my_error::e1});
            },

            []( leaf::match_value<e_my_error, my_error::e2> )
            {
                return 1;
            },

            []
            {
                return 2;
            } );
        BOOST_TEST_EQ(r, 2);
    }

    return boost::report_errors();
}
