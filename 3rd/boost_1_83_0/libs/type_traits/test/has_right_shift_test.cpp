//  (C) Copyright 2009-2011 Frederic Bron.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifdef TEST_STD
#  include <type_traits>
#else
#  include <boost/type_traits/has_right_shift.hpp>
#endif
#include "test.hpp"
#include "check_integral_constant.hpp"

#define BOOST_TT_TRAIT_NAME has_right_shift
#define BOOST_TT_TRAIT_OP >>

#include <istream>
#include <string>

#include "has_binary_operators.hpp"

BOOST_TT_PROC void specific() {
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const, void, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool &, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const &, void, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void, int &, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void, double, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void, void* const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* const, void, int >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int*, void, int >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const &, void, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool, bool, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool, bool &, int const >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool, int const, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool, int & >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool, double &, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool, double const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool, int* const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool, int* &, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool, int* &, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool, int* const &, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool, int* const &, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool, int* const &, int const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const, bool const >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const, bool const, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const, int const, int >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const, int &, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const, double const, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const, double &, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const, double const &, bool >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const, double const &, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const, void* const &, int >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const, void* const &, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const, int* >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const, int* const, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool &, bool, bool const & >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool &, int const, bool const & >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool &, int & >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool &, int const &, int const >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool &, void* &, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool &, void* &, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool &, int*, int const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool &, int* const &, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const &, bool, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const &, bool &, bool >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const &, double const, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const &, double const, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const &, void* const, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const &, void* &, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const &, void* &, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const &, void* const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< bool const &, void* const &, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int, bool, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int, bool const, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int, bool const, int >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int, bool &, bool >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int, bool &, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int, bool &, int const & >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int, double &, int >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int, void* const, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int, void* const &, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int, int* & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int, int* const &, int const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const, bool &, int >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const, bool &, int const & >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const, bool const &, bool const >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const, int, bool const >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const, int const, bool const & >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const, double const, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const, double & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const, double &, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const, void* const, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const, void* &, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const, int*, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const, int* &, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const, int* &, int const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int &, bool &, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int &, bool const &, bool const & >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int &, bool const &, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int &, int &, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int &, double, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int &, double, int >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int &, double const, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int &, double const, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int &, void* &, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int &, int* &, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const &, int const, int const & >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const &, int &, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const &, int const &, bool >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const &, double, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const &, double const, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const &, void* const, int const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const &, void* &, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const &, void* &, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const &, int* &, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int const &, int* const &, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double, bool const, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double, int, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double, int &, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double, double, bool >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double, double const, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double, void* const, int const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double, void* const &, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const, bool &, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const, bool const &, int const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const, int, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const, int, int const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const, double, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const, void*, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const, void* &, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const, int* const &, int const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double &, bool, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double &, bool const &, int >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double &, double, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double &, double const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double &, double &, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double &, double &, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double &, double const &, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double &, void* const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double &, int*, int const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const &, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const &, int, int const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const &, int &, bool >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const &, int &, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const &, void*, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const &, void* const, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const &, void* const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const &, int* const, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const &, int* &, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< double const &, int* &, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void*, bool, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void*, bool const, int >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void*, int, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void*, int const, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void*, int const, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void*, int* const, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void*, int* const &, int >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* const, bool >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* const, int const, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* const, double const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* const, void* const, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* const, void* const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* &, bool const &, bool >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* &, int const, int >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* &, int const, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* &, double, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* &, double, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* &, double const, bool >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* &, double const, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* &, void* const, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* &, int* const, bool >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* &, int* const, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* &, int* const &, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* &, int* const &, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* const &, bool, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* const &, bool const, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* const &, bool const, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* const &, int >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* const &, int, int >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* const &, int const, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* const &, int &, bool >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* const &, int &, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* const &, double const, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* const &, void* &, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< void* const &, void* const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int*, int, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int*, int const, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int*, int const, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int*, int &, int >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int*, double const &, bool >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int*, void*, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int*, int* const, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int*, int* &, int >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int*, int* &, int const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int*, int* const &, bool >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const, bool, int const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const, double const, bool >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const, double &, int >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const, double const &, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const, void* >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const, int* const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const, int* &, int >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* &, double &, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* &, void* const &, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* &, int*, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* &, int* const &, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const &, bool, bool const >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const &, bool, int >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const &, bool, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const &, int, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const &, int, int >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const &, int &, bool & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const &, int &, int const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const &, double const, int const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const &, void*, bool >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const &, void* const &, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const &, int*, bool const & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const &, int* const, void >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const &, int* &, int & >::value), 0);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< int* const &, int* const &, int const & >::value), 0);

   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< std::istream, bool&, std::istream& >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< std::istream, short&, std::istream& >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< std::istream, unsigned short&, std::istream& >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< std::istream, int&, std::istream& >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< std::istream, unsigned int&, std::istream& >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< std::istream, long&, std::istream& >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< std::istream, unsigned long&, std::istream& >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< std::istream, float&, std::istream& >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< std::istream, double&, std::istream& >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< std::istream, void*&, std::istream& >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< std::istream, char&, std::istream& >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< std::istream, signed char&, std::istream& >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< std::istream, unsigned char&, std::istream& >::value), 1);
#if !(defined(_LIBCPP_VERSION) && (_LIBCPP_VERSION >= 9000)) && !defined(_HAS_CXX20)
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< std::istream, char*, std::istream& >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< std::istream, signed char*, std::istream& >::value), 1);
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< std::istream, unsigned char*, std::istream& >::value), 1);
#endif
   BOOST_CHECK_INTEGRAL_CONSTANT((::boost::BOOST_TT_TRAIT_NAME< std::istream, std::string&, std::istream& >::value), 1);
}

TT_TEST_BEGIN(BOOST_TT_TRAIT_NAME)
   common();
   specific();
TT_TEST_END
