// Copyright (c) 2007 Joseph Gauterin
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/utility/swap.hpp>
#include <boost/core/lightweight_test.hpp>
#define BOOST_CHECK BOOST_TEST
#define BOOST_CHECK_EQUAL BOOST_TEST_EQ

//Put test class in namespace boost
namespace boost
{
  #include "./swap_test_class.hpp"
}

//Provide swap function in namespace boost
namespace boost
{
  void swap(swap_test_class& left, swap_test_class& right)
  {
    left.swap(right);
  }
}

int main()
{
  const boost::swap_test_class initial_value1(1);
  const boost::swap_test_class initial_value2(2);

  boost::swap_test_class object1 = initial_value1;
  boost::swap_test_class object2 = initial_value2;

  boost::swap_test_class::reset();
  boost::swap(object1,object2);

  BOOST_CHECK(object1 == initial_value2);
  BOOST_CHECK(object2 == initial_value1);

  BOOST_CHECK_EQUAL(boost::swap_test_class::swap_count(),1);
  BOOST_CHECK_EQUAL(boost::swap_test_class::copy_count(),0);

  return boost::report_errors();
}

