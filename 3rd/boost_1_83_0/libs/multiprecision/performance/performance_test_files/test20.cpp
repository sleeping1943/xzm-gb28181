///////////////////////////////////////////////////////////////
//  Copyright 2019 John Maddock. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt

#include "../performance_test.hpp"
#if defined(TEST_CPP_INT_RATIONAL)
#include <boost/multiprecision/cpp_int.hpp>
#endif

void test20()
{
#ifdef TEST_CPP_INT_RATIONAL
   test<boost::multiprecision::cpp_rational>("cpp_rational", 1024);
#endif
}
