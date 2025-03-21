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

#if defined(BOOST_NO_STRINGSTREAM) || defined(BOOST_NO_STD_WSTRING)
#define BOOST_LCAST_NO_WCHAR_T
#endif

template <class CharT>
void test_impl(const CharT* wc_arr)
{
    typedef CharT                       wide_char;
    typedef std::basic_string<CharT>    wide_string;
    const char c_arr[]            = "Test array of chars";
    const unsigned char uc_arr[]  = "Test array of chars";
    const signed char sc_arr[]    = "Test array of chars";

    // Following tests depend on realization of std::locale
    // and pass for popular compilers and STL realizations
    BOOST_CHECK(boost::lexical_cast<wide_char>(c_arr[0]) == wc_arr[0]);
    BOOST_CHECK(boost::lexical_cast<wide_string>(c_arr) == wide_string(wc_arr));

    BOOST_CHECK(boost::lexical_cast<wide_string>(sc_arr) == wide_string(wc_arr) );
    BOOST_CHECK(boost::lexical_cast<wide_string>(uc_arr) == wide_string(wc_arr) );

    BOOST_CHECK(boost::lexical_cast<wide_char>(uc_arr[0]) == wc_arr[0]);
    BOOST_CHECK(boost::lexical_cast<wide_char>(sc_arr[0]) == wc_arr[0]);
}


void test_char_types_conversions_wchar_t()
{
#ifndef BOOST_LCAST_NO_WCHAR_T
    test_impl(L"Test array of chars");
    wchar_t c = boost::detail::lcast_char_constants<wchar_t>::zero;
    BOOST_CHECK(L'0' == c);

    c = boost::detail::lcast_char_constants<wchar_t>::minus;
    BOOST_CHECK(L'-' == c);

    c = boost::detail::lcast_char_constants<wchar_t>::plus;
    BOOST_CHECK(L'+' == c);

    c = boost::detail::lcast_char_constants<wchar_t>::lowercase_e;
    BOOST_CHECK(L'e' == c);

    c = boost::detail::lcast_char_constants<wchar_t>::capital_e;
    BOOST_CHECK(L'E' == c);

    c = boost::detail::lcast_char_constants<wchar_t>::c_decimal_separator;
    BOOST_CHECK(L'.' == c);
#endif

    BOOST_CHECK(true);
}

void test_char_types_conversions_char16_t()
{
#if !defined(BOOST_NO_CXX11_CHAR16_T) && !defined(BOOST_NO_CXX11_UNICODE_LITERALS) && defined(BOOST_STL_SUPPORTS_NEW_UNICODE_LOCALES)
    test_impl(u"Test array of chars");
    char16_t c = boost::detail::lcast_char_constants<char16_t>::zero;
    BOOST_CHECK(u'0' == c);

    c = boost::detail::lcast_char_constants<char16_t>::minus;
    BOOST_CHECK(u'-' == c);

    c = boost::detail::lcast_char_constants<char16_t>::plus;
    BOOST_CHECK(u'+' == c);

    c = boost::detail::lcast_char_constants<char16_t>::lowercase_e;
    BOOST_CHECK(u'e' == c);

    c = boost::detail::lcast_char_constants<char16_t>::capital_e;
    BOOST_CHECK(u'E' == c);

    c = boost::detail::lcast_char_constants<char16_t>::c_decimal_separator;
    BOOST_CHECK(u'.' == c);
#endif

    BOOST_CHECK(true);
}

void test_char_types_conversions_char32_t()
{
#if !defined(BOOST_NO_CXX11_CHAR32_T) && !defined(BOOST_NO_CXX11_UNICODE_LITERALS) && defined(BOOST_STL_SUPPORTS_NEW_UNICODE_LOCALES)
    test_impl(U"Test array of chars");
    char32_t c = boost::detail::lcast_char_constants<char32_t>::zero;
    BOOST_CHECK(U'0' == c);

    c = boost::detail::lcast_char_constants<char32_t>::minus;
    BOOST_CHECK(U'-' == c);

    c = boost::detail::lcast_char_constants<char32_t>::plus;
    BOOST_CHECK(U'+' == c);

    c = boost::detail::lcast_char_constants<char32_t>::lowercase_e;
    BOOST_CHECK(U'e' == c);

    c = boost::detail::lcast_char_constants<char32_t>::capital_e;
    BOOST_CHECK(U'E', == c);

    c = boost::detail::lcast_char_constants<char32_t>::c_decimal_separator;
    BOOST_CHECK(U'.' == c);
#endif

    BOOST_CHECK(true);
}

unit_test::test_suite *init_unit_test_suite(int, char *[])
{
    unit_test::test_suite *suite =
        BOOST_TEST_SUITE("lexical_cast char => wide characters unit test (widening test)");
    suite->add(BOOST_TEST_CASE(&test_char_types_conversions_wchar_t));
    suite->add(BOOST_TEST_CASE(&test_char_types_conversions_char16_t));
    suite->add(BOOST_TEST_CASE(&test_char_types_conversions_char32_t));

    return suite;
}
