// Copyright 2018-2022 Emil Dotchevski and Reverge Studios, Inc.

// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/leaf/handle_errors.hpp>

namespace leaf = boost::leaf;

leaf::diagnostic_info f();
leaf::diagnostic_info a = f();
leaf::diagnostic_info b = a;
