//  Unit test for boost::lexical_cast.
//
//  See http://www.boost.org for most recent version, including documentation.
//
//  Copyright Antony Polukhin, 2011-2023.
//
//  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).

#include <boost/config.hpp>

#if defined(__INTEL_COMPILER)
#pragma warning(disable: 193 383 488 981 1418 1419)
#elif defined(BOOST_MSVC)
#pragma warning(disable: 4097 4100 4121 4127 4146 4244 4245 4511 4512 4701 4800)
#endif

#include <boost/lexical_cast.hpp>

#include <boost/test/unit_test.hpp>
using namespace boost;

void test_typedefed_wchar_t(unsigned short)  // wchar_t is a typedef for unsigned short
{
    BOOST_CHECK(boost::lexical_cast<int>(L'A') == 65);
    BOOST_CHECK(boost::lexical_cast<int>(L'B') == 66);

    BOOST_CHECK(boost::lexical_cast<wchar_t>(L"65") == 65);
    BOOST_CHECK(boost::lexical_cast<wchar_t>(L"66") == 66);
}

template <class T>
void test_typedefed_wchar_t(T)
{
    BOOST_CHECK(1);
}



void test_typedefed_wchar_t_runtime()
{
    test_typedefed_wchar_t(L'0');
}

unit_test::test_suite *init_unit_test_suite(int, char *[])
{
    unit_test::test_suite *suite =
        BOOST_TEST_SUITE("lexical_cast typedefed wchar_t runtime test");
    suite->add(BOOST_TEST_CASE(&test_typedefed_wchar_t_runtime));

    return suite;
}
