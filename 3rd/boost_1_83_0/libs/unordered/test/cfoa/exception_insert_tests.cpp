// Copyright (C) 2023 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "exception_helpers.hpp"

#include <boost/unordered/concurrent_flat_map.hpp>

#include <boost/core/ignore_unused.hpp>

namespace {
  test::seed_t initialize_seed(73987);

  struct lvalue_inserter_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      enable_exceptions();

      std::atomic<std::uint64_t> num_inserts{0};
      thread_runner(values, [&x, &num_inserts](boost::span<T> s) {
        for (auto const& r : s) {
          try {
            bool b = x.insert(r);
            if (b) {
              ++num_inserts;
            }
          } catch (...) {
          }
        }
      });

      disable_exceptions();

      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
    }
  } lvalue_inserter;

  struct norehash_lvalue_inserter_type : public lvalue_inserter_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      x.reserve(values.size());
      lvalue_inserter_type::operator()(values, x);
      BOOST_TEST_GT(raii::copy_constructor, 0u);
      BOOST_TEST_EQ(raii::move_constructor, 0u);
    }
  } norehash_lvalue_inserter;

  struct rvalue_inserter_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      BOOST_TEST_EQ(raii::copy_constructor, 0u);

      enable_exceptions();

      std::atomic<std::uint64_t> num_inserts{0};
      thread_runner(values, [&x, &num_inserts](boost::span<T> s) {
        for (auto& r : s) {
          try {
            bool b = x.insert(std::move(r));
            if (b) {
              ++num_inserts;
            }
          } catch (...) {
          }
        }
      });

      disable_exceptions();

      if (!std::is_same<T, typename X::value_type>::value) {
        BOOST_TEST_EQ(raii::copy_constructor, 0u);
      }

      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
    }
  } rvalue_inserter;

  struct norehash_rvalue_inserter_type : public rvalue_inserter_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      x.reserve(values.size());

      BOOST_TEST_EQ(raii::copy_constructor, 0u);
      BOOST_TEST_EQ(raii::move_constructor, 0u);

      rvalue_inserter_type::operator()(values, x);

      if (std::is_same<T, typename X::value_type>::value) {
        BOOST_TEST_EQ(raii::copy_constructor, x.size());
        BOOST_TEST_EQ(raii::move_constructor, x.size());
      } else {
        BOOST_TEST_EQ(raii::copy_constructor, 0u);
        BOOST_TEST_EQ(raii::move_constructor, 2 * x.size());
      }
    }
  } norehash_rvalue_inserter;

  struct iterator_range_inserter_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      for (std::size_t i = 0; i < 10; ++i) {
        x.insert(values[i]);
      }

      enable_exceptions();
      thread_runner(values, [&x](boost::span<T> s) {
        try {
          x.insert(s.begin(), s.end());
        } catch (...) {
        }
      });
      disable_exceptions();

      BOOST_TEST_EQ(raii::copy_assignment, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
    }
  } iterator_range_inserter;

  struct lvalue_insert_or_assign_copy_assign_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      enable_exceptions();
      thread_runner(values, [&x](boost::span<T> s) {
        for (auto& r : s) {
          try {
            x.insert_or_assign(r.first, r.second);
          } catch (...) {
          }
        }
      });
      disable_exceptions();

      BOOST_TEST_EQ(raii::default_constructor, 0u);
      BOOST_TEST_GT(raii::copy_constructor, 0u);
      BOOST_TEST_GT(raii::move_constructor, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
    }
  } lvalue_insert_or_assign_copy_assign;

  struct lvalue_insert_or_assign_move_assign_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      enable_exceptions();
      thread_runner(values, [&x](boost::span<T> s) {
        for (auto& r : s) {
          try {

            x.insert_or_assign(r.first, std::move(r.second));
          } catch (...) {
          }
        }
      });
      disable_exceptions();

      BOOST_TEST_EQ(raii::default_constructor, 0u);
      BOOST_TEST_GT(raii::copy_constructor, 0u);
      BOOST_TEST_GT(raii::move_constructor, 0u);
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
    }
  } lvalue_insert_or_assign_move_assign;

  struct rvalue_insert_or_assign_copy_assign_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      enable_exceptions();
      thread_runner(values, [&x](boost::span<T> s) {
        for (auto& r : s) {
          try {
            x.insert_or_assign(std::move(r.first), r.second);
          } catch (...) {
          }
        }
      });
      disable_exceptions();

      BOOST_TEST_EQ(raii::default_constructor, 0u);
      BOOST_TEST_GT(raii::copy_constructor, 0u);
      BOOST_TEST_GT(raii::move_constructor, x.size()); // rehashing
      BOOST_TEST_EQ(raii::move_assignment, 0u);
    }
  } rvalue_insert_or_assign_copy_assign;

  struct rvalue_insert_or_assign_move_assign_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      enable_exceptions();
      thread_runner(values, [&x](boost::span<T> s) {
        for (auto& r : s) {
          try {
            x.insert_or_assign(std::move(r.first), std::move(r.second));
          } catch (...) {
          }
        }
      });
      disable_exceptions();

      BOOST_TEST_EQ(raii::default_constructor, 0u);
      BOOST_TEST_EQ(raii::copy_constructor, 0u);
      BOOST_TEST_GT(raii::move_constructor, 0u);
      BOOST_TEST_EQ(raii::copy_assignment, 0u);
    }
  } rvalue_insert_or_assign_move_assign;

  struct lvalue_insert_or_cvisit_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      std::atomic<std::uint64_t> num_inserts{0};

      enable_exceptions();
      thread_runner(values, [&x, &num_inserts](boost::span<T> s) {
        for (auto& r : s) {
          try {
            bool b = x.insert_or_cvisit(
              r, [](typename X::value_type const& v) { (void)v; });

            if (b) {
              ++num_inserts;
            }
          } catch (...) {
          }
        }
      });
      disable_exceptions();

      BOOST_TEST_GT(num_inserts, 0u);
      BOOST_TEST_EQ(raii::default_constructor, 0u);
      // don't check move construction count here because of rehashing
      BOOST_TEST_GT(raii::move_constructor, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
    }
  } lvalue_insert_or_cvisit;

  struct lvalue_insert_or_visit_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      std::atomic<std::uint64_t> num_inserts{0};

      enable_exceptions();
      thread_runner(values, [&x, &num_inserts](boost::span<T> s) {
        for (auto& r : s) {
          try {
            bool b =
              x.insert_or_visit(r, [](typename X::value_type& v) { (void)v; });

            if (b) {
              ++num_inserts;
            }
          } catch (...) {
          }
        }
      });
      disable_exceptions();

      BOOST_TEST_GT(num_inserts, 0u);

      BOOST_TEST_EQ(raii::default_constructor, 0u);

      // don't check move construction count here because of rehashing
      BOOST_TEST_GT(raii::move_constructor, 0u);
      BOOST_TEST_EQ(raii::move_assignment, 0u);
    }
  } lvalue_insert_or_visit;

  struct rvalue_insert_or_cvisit_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      std::atomic<std::uint64_t> num_inserts{0};

      enable_exceptions();
      thread_runner(values, [&x, &num_inserts](boost::span<T> s) {
        for (auto& r : s) {
          try {
            bool b = x.insert_or_cvisit(
              std::move(r), [](typename X::value_type const& v) { (void)v; });

            if (b) {
              ++num_inserts;
            }
          } catch (...) {
          }
        }
      });
      disable_exceptions();

      BOOST_TEST_GT(num_inserts, 0u);

      BOOST_TEST_EQ(raii::default_constructor, 0u);
    }
  } rvalue_insert_or_cvisit;

  struct rvalue_insert_or_visit_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      std::atomic<std::uint64_t> num_inserts{0};

      enable_exceptions();
      thread_runner(values, [&x, &num_inserts](boost::span<T> s) {
        for (auto& r : s) {
          try {
            bool b = x.insert_or_visit(
              std::move(r), [](typename X::value_type& v) { (void)v; });

            if (b) {
              ++num_inserts;
            }
          } catch (...) {
          }
        }
      });
      disable_exceptions();

      BOOST_TEST_GT(num_inserts, 0u);

      BOOST_TEST_EQ(raii::default_constructor, 0u);
      if (!std::is_same<T, typename X::value_type>::value) {
        BOOST_TEST_EQ(raii::copy_constructor, 0u);
      }
    }
  } rvalue_insert_or_visit;

  struct iterator_range_insert_or_cvisit_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      for (std::size_t i = 0; i < 10; ++i) {
        x.insert(values[i]);
      }

      enable_exceptions();
      thread_runner(values, [&x](boost::span<T> s) {
        try {
          x.insert_or_cvisit(s.begin(), s.end(),
            [](typename X::value_type const& v) { (void)v; });
        } catch (...) {
        }
      });
      disable_exceptions();

      BOOST_TEST_EQ(raii::default_constructor, 0u);
    }
  } iterator_range_insert_or_cvisit;

  struct iterator_range_insert_or_visit_type
  {
    template <class T, class X> void operator()(std::vector<T>& values, X& x)
    {
      for (std::size_t i = 0; i < 10; ++i) {
        x.insert(values[i]);
      }

      enable_exceptions();
      thread_runner(values, [&x](boost::span<T> s) {
        try {
          x.insert_or_visit(s.begin(), s.end(),
            [](typename X::value_type const& v) { (void)v; });
        } catch (...) {
        }
      });
      disable_exceptions();

      BOOST_TEST_EQ(raii::default_constructor, 0u);
    }
  } iterator_range_insert_or_visit;

  template <class X, class G, class F>
  void insert(X*, G gen, F inserter, test::random_generator rg)
  {
    disable_exceptions();

    auto values = make_random_values(1024 * 16, [&] { return gen(rg); });
    auto reference_map =
      boost::unordered_flat_map<raii, raii>(values.begin(), values.end());

    raii::reset_counts();
    {
      X x;

      inserter(values, x);

      test_fuzzy_matches_reference(x, reference_map, rg);
    }
    check_raii_counts();
  }

  boost::unordered::concurrent_flat_map<raii, raii, stateful_hash,
    stateful_key_equal, stateful_allocator<std::pair<raii const, raii> > >* map;

} // namespace

using test::default_generator;
using test::limited_range;
using test::sequential;

// clang-format off
UNORDERED_TEST(
  insert,
  ((map))
  ((exception_value_type_generator)(exception_init_type_generator))
  ((lvalue_inserter)(rvalue_inserter)(iterator_range_inserter)
   (norehash_lvalue_inserter)(norehash_rvalue_inserter)
   (lvalue_insert_or_cvisit)(lvalue_insert_or_visit)
   (rvalue_insert_or_cvisit)(rvalue_insert_or_visit)
   (iterator_range_insert_or_cvisit)(iterator_range_insert_or_visit))
  ((default_generator)(sequential)(limited_range)))

UNORDERED_TEST(
  insert,
  ((map))
  ((exception_init_type_generator))
  ((lvalue_insert_or_assign_copy_assign)(lvalue_insert_or_assign_move_assign)
   (rvalue_insert_or_assign_copy_assign)(rvalue_insert_or_assign_move_assign))
  ((default_generator)(sequential)(limited_range)))

// clang-format on

RUN_TESTS()
