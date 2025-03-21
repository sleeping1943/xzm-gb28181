//
// experimental/coro/partial.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2021-2023 Klemens D. Morgenstern
//                         (klemens dot morgenstern at gmx dot net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include <boost/asio/experimental/coro.hpp>

#include <boost/asio/io_context.hpp>
#include "../../unit_test.hpp"

namespace exp =  boost::asio::experimental;

namespace coro {

template<typename Value = void>
struct tracked_allocator
{
  using value_type = Value;
  std::vector<std::pair<void*, std::size_t>>   & allocs, &deallocs;

  tracked_allocator(std::vector<std::pair<void*, std::size_t>>   & allocs,
                    std::vector<std::pair<void*, std::size_t>>   & deallocs) : allocs(allocs), deallocs(deallocs) {}


  template<typename T>
  tracked_allocator(const tracked_allocator<T> & a) : allocs(a.allocs), deallocs(a.deallocs) {}

  value_type* allocate(std::size_t n)
  {
    auto p = new char[n * sizeof(Value)];
    allocs.emplace_back(p, n);
    return reinterpret_cast<value_type*>(p);
  }

  void deallocate(void* p, std::size_t n)
  {
    deallocs.emplace_back(p, n);
    delete[] static_cast<char*>(p);
//    BOOST_ASIO_CHECK(allocs.back() == deallocs.back());
  }

  bool operator==(const tracked_allocator & rhs) const
  {
    return &allocs == &rhs.allocs
           && &deallocs == &rhs.deallocs;
  }
};

exp::coro<void, void, boost::asio::any_io_executor, tracked_allocator<void>>
        alloc_test_impl(boost::asio::io_context & ctx, int, std::allocator_arg_t, tracked_allocator<void> ta, double)
{
  co_return ;
}

void alloc_test()
{
  std::vector<std::pair<void*, std::size_t>> allocs, deallocs;
  boost::asio::io_context ctx;
  bool ran = false;

  {
    auto pp = alloc_test_impl(ctx, 42, std::allocator_arg, {allocs, deallocs}, 42.);

    BOOST_ASIO_CHECK(allocs.size()  == 1u);
    BOOST_ASIO_CHECK(deallocs.empty());

    pp.async_resume([&](auto e){ran = true; BOOST_ASIO_CHECK(!e);});
    ctx.run();
    BOOST_ASIO_CHECK(deallocs.size() == 0u);
  }
  ctx.restart();
  ctx.run();
  BOOST_ASIO_CHECK(deallocs.size() == 1u);
  BOOST_ASIO_CHECK(allocs == deallocs);

  BOOST_ASIO_CHECK(ran);

  ran = false;

  auto p = boost::asio::experimental::detail::post_coroutine(
          ctx,
          boost::asio::bind_allocator(tracked_allocator{allocs, deallocs}, [&]{ran = true;})).handle;
  BOOST_ASIO_CHECK(allocs.size()  == 2u);
  BOOST_ASIO_CHECK(deallocs.size() == 1u);
  p.resume();
  BOOST_ASIO_CHECK(allocs.size()  == 3u);
  BOOST_ASIO_CHECK(deallocs.size() == 2u);
  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(allocs == deallocs);
}

} // namespace coro

BOOST_ASIO_TEST_SUITE
(
  "coro/allocate",
  BOOST_ASIO_TEST_CASE(::coro::alloc_test)
)
