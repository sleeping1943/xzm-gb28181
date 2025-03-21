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


#include <boost/core/cmath.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>

#if defined(BOOST_NO_STRINGSTREAM) || defined(BOOST_NO_STD_WSTRING)
#define BOOST_LCAST_NO_WCHAR_T
#endif

using namespace boost;

template <class T>
bool is_pos_inf(T value)
{
    return (boost::core::isinf)(value) && !(boost::core::signbit)(value);
}

template <class T>
bool is_neg_inf(T value)
{
    return (boost::core::isinf)(value) && (boost::core::signbit)(value);
}

template <class T>
bool is_pos_nan(T value)
{
    return (boost::core::isnan)(value) && !(boost::core::signbit)(value);
}

template <class T>
bool is_neg_nan(T value)
{
    /* There is some strange behaviour on Itanium platform with -nan nuber for long double.
    * It is a IA64 feature, or it is a boost::math feature, not a lexical_cast bug */
#if defined(__ia64__) || defined(_M_IA64)
    return (boost::core::isnan)(value)
            && ( boost::is_same<T, long double >::value || (boost::core::signbit)(value) );
#else
    return (boost::core::isnan)(value) && (boost::core::signbit)(value);
#endif
}

template <class T>
void test_inf_nan_templated()
{
    typedef T test_t;

    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>("inf") ) );
    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>("INF") ) );

    BOOST_CHECK( is_neg_inf( lexical_cast<test_t>("-inf") ) );
    BOOST_CHECK( is_neg_inf( lexical_cast<test_t>("-INF") ) );

    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>("+inf") ) );
    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>("+INF") ) );

    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>("infinity") ) );
    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>("INFINITY") ) );

    BOOST_CHECK( is_neg_inf( lexical_cast<test_t>("-infinity") ) );
    BOOST_CHECK( is_neg_inf( lexical_cast<test_t>("-INFINITY") ) );

    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>("+infinity") ) );
    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>("+INFINITY") ) );

    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>("iNfiNity") ) );
    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>("INfinity") ) );

    BOOST_CHECK( is_neg_inf( lexical_cast<test_t>("-inFINITY") ) );
    BOOST_CHECK( is_neg_inf( lexical_cast<test_t>("-INFINITY") ) );

    BOOST_CHECK( is_pos_nan( lexical_cast<test_t>("nan") ) );
    BOOST_CHECK( is_pos_nan( lexical_cast<test_t>("NAN") ) );

    BOOST_CHECK( is_neg_nan( lexical_cast<test_t>("-nan") ) );
    BOOST_CHECK( is_neg_nan( lexical_cast<test_t>("-NAN") ) );

    BOOST_CHECK( is_pos_nan( lexical_cast<test_t>("+nan") ) );
    BOOST_CHECK( is_pos_nan( lexical_cast<test_t>("+NAN") ) );

    BOOST_CHECK( is_pos_nan( lexical_cast<test_t>("nAn") ) );
    BOOST_CHECK( is_pos_nan( lexical_cast<test_t>("NaN") ) );

    BOOST_CHECK( is_neg_nan( lexical_cast<test_t>("-nAn") ) );
    BOOST_CHECK( is_neg_nan( lexical_cast<test_t>("-NaN") ) );

    BOOST_CHECK( is_pos_nan( lexical_cast<test_t>("+Nan") ) );
    BOOST_CHECK( is_pos_nan( lexical_cast<test_t>("+nAN") ) );

    BOOST_CHECK( is_pos_nan( lexical_cast<test_t>("nan()") ) );
    BOOST_CHECK( is_pos_nan( lexical_cast<test_t>("NAN(some string)") ) );
    BOOST_CHECK_THROW( lexical_cast<test_t>("NAN(some string"), bad_lexical_cast );

    BOOST_CHECK(lexical_cast<std::string>( (boost::core::copysign)(std::numeric_limits<test_t >::infinity(), static_cast<test_t>(-1.0)))
                == "-inf" );
    BOOST_CHECK(lexical_cast<std::string>( std::numeric_limits<test_t >::infinity()) == "inf" );
    BOOST_CHECK(lexical_cast<std::string>( std::numeric_limits<test_t >::quiet_NaN()) == "nan" );
#if !defined(__ia64__) && !defined(_M_IA64)
    BOOST_CHECK(lexical_cast<std::string>(
                (boost::core::copysign)(std::numeric_limits<test_t >::quiet_NaN(), static_cast<test_t>(-1.0)))
                == "-nan" );
#endif

#ifndef  BOOST_LCAST_NO_WCHAR_T
    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>(L"inf") ) );
    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>(L"INF") ) );

    BOOST_CHECK( is_neg_inf( lexical_cast<test_t>(L"-inf") ) );
    BOOST_CHECK( is_neg_inf( lexical_cast<test_t>(L"-INF") ) );

    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>(L"+inf") ) );
    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>(L"+INF") ) );

    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>(L"infinity") ) );
    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>(L"INFINITY") ) );

    BOOST_CHECK( is_neg_inf( lexical_cast<test_t>(L"-infinity") ) );
    BOOST_CHECK( is_neg_inf( lexical_cast<test_t>(L"-INFINITY") ) );

    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>(L"+infinity") ) );
    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>(L"+INFINITY") ) );

    BOOST_CHECK( is_neg_inf( lexical_cast<test_t>(L"-infINIty") ) );
    BOOST_CHECK( is_neg_inf( lexical_cast<test_t>(L"-INFiniTY") ) );

    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>(L"+inFINIty") ) );
    BOOST_CHECK( is_pos_inf( lexical_cast<test_t>(L"+INfinITY") ) );

    BOOST_CHECK( is_pos_nan( lexical_cast<test_t>(L"nan") ) );
    BOOST_CHECK( is_pos_nan( lexical_cast<test_t>(L"NAN") ) );

    BOOST_CHECK( is_neg_nan( lexical_cast<test_t>(L"-nan") ) );
    BOOST_CHECK( is_neg_nan( lexical_cast<test_t>(L"-NAN") ) );

    BOOST_CHECK( is_pos_nan( lexical_cast<test_t>(L"+nan") ) );
    BOOST_CHECK( is_pos_nan( lexical_cast<test_t>(L"+NAN") ) );

    BOOST_CHECK( is_pos_nan( lexical_cast<test_t>(L"nan()") ) );
    BOOST_CHECK( is_pos_nan( lexical_cast<test_t>(L"NAN(some string)") ) );
    BOOST_CHECK_THROW( lexical_cast<test_t>(L"NAN(some string"), bad_lexical_cast );

    BOOST_CHECK(lexical_cast<std::wstring>( (boost::core::copysign)(std::numeric_limits<test_t >::infinity(), static_cast<test_t>(-1.0)))
                == L"-inf" );
    BOOST_CHECK(lexical_cast<std::wstring>( std::numeric_limits<test_t >::infinity()) == L"inf" );
    BOOST_CHECK(lexical_cast<std::wstring>( std::numeric_limits<test_t >::quiet_NaN()) == L"nan" );
#if !defined(__ia64__) && !defined(_M_IA64)
    BOOST_CHECK(lexical_cast<std::wstring>(
                (boost::core::copysign)(std::numeric_limits<test_t >::quiet_NaN(), static_cast<test_t>(-1.0)))
                == L"-nan" );
#endif

#endif
}

void test_inf_nan_float()
{
    test_inf_nan_templated<float >();
}

void test_inf_nan_double()
{
    test_inf_nan_templated<double >();
}

void test_inf_nan_long_double()
{
// We do not run tests on compilers with bugs
#ifndef BOOST_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS
    test_inf_nan_templated<long double >();
#endif
    BOOST_CHECK(true);
}

unit_test::test_suite *init_unit_test_suite(int, char *[])
{
    unit_test::test_suite *suite =
        BOOST_TEST_SUITE("lexical_cast inf anf nan parsing unit test");
    suite->add(BOOST_TEST_CASE(&test_inf_nan_float));
    suite->add(BOOST_TEST_CASE(&test_inf_nan_double));
    suite->add(BOOST_TEST_CASE(&test_inf_nan_long_double));

    return suite;
}
