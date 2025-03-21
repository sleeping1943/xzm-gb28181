// Copyright 2018-2022 Emil Dotchevski and Reverge Studios, Inc.

// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/leaf/handle_errors.hpp>
#include <boost/leaf/pred.hpp>
#include <boost/leaf/result.hpp>

namespace leaf = boost::leaf;

int main()
{
    return leaf::try_handle_all(
        []() -> leaf::result<int>
        {
            return 0;
        },
        []( leaf::match<int,4> const & ) // leaf::match<> must be taken by value
        {
            return 1;
        },
        []
        {
            return 2;
        });
    return 0;
}
