
// Copyright 2016 Daniel James.
// Copyright 2023 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "../helpers/postfix.hpp"
#include "../helpers/prefix.hpp"
#include "../helpers/unordered.hpp"

#include "../helpers/helpers.hpp"
#include "../helpers/metafunctions.hpp"
#include "../helpers/test.hpp"
#include <boost/core/pointer_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <set>
#include <string>

template <template <class Key, class T, class Hash = boost::hash<Key>,
  class Pred = std::equal_to<Key>,
  class Allocator = std::allocator<std::pair<Key const, T> > >
  class Map>
static void example1()
{
  typedef typename Map<int, std::string>::insert_return_type insert_return_type;

  Map<int, std::string> src;
  src.emplace(1, "one");
  src.emplace(2, "two");
  src.emplace(3, "buckle my shoe");
  Map<int, std::string> dst;
  dst.emplace(3, "three");

  dst.insert(src.extract(src.find(1)));
  dst.insert(src.extract(2));
  insert_return_type r = dst.insert(src.extract(3));

  BOOST_TEST(src.empty());
  BOOST_TEST(dst.size() == 3);
  BOOST_TEST(dst[1] == "one");
  BOOST_TEST(dst[2] == "two");
  BOOST_TEST(dst[3] == "three");
  BOOST_TEST(!r.inserted);
  BOOST_TEST(r.position == dst.find(3));
  BOOST_TEST(r.node.mapped() == "buckle my shoe");
}

template <template <class Key, class Hash = boost::hash<Key>,
  class Pred = std::equal_to<Key>, class Allocator = std::allocator<Key> >
  class Set>
static void example2()
{
  Set<int> src;
  src.insert(1);
  src.insert(3);
  src.insert(5);
  Set<int> dst;
  dst.insert(2);
  dst.insert(4);
  dst.insert(5);
  // dst.merge(src);
  // Merge src into dst.
  // src == {5}
  // dst == {1, 2, 3, 4, 5}
}

template <template <class Key, class Hash = boost::hash<Key>,
  class Pred = std::equal_to<Key>, class Allocator = std::allocator<Key> >
  class Set>
static void example3()
{
  typedef typename Set<int>::iterator iterator;

  Set<int> src;
  src.insert(1);
  src.insert(3);
  src.insert(5);
  Set<int> dst;
  dst.insert(2);
  dst.insert(4);
  dst.insert(5);
  for (iterator i = src.begin(); i != src.end();) {
    std::pair<iterator, iterator> p = dst.equal_range(*i);
    if (p.first == p.second)
      dst.insert(p.first, src.extract(i++));
    else
      ++i;
  }
  BOOST_TEST(src.size() == 1);
  BOOST_TEST(*src.begin() == 5);

  std::set<int> dst2(dst.begin(), dst.end());
  std::set<int>::iterator it = dst2.begin();
  BOOST_TEST(*it++ == 1);
  BOOST_TEST(*it++ == 2);
  BOOST_TEST(*it++ == 3);
  BOOST_TEST(*it++ == 4);
  BOOST_TEST(*it++ == 5);
  BOOST_TEST(it == dst2.end());
}

template <template <class Key, class T, class Hash = boost::hash<Key>,
            class Pred = std::equal_to<Key>,
            class Allocator = std::allocator<std::pair<Key const, T> > >
          class Map,
  template <class Key, class Hash = boost::hash<Key>,
    class Pred = std::equal_to<Key>, class Allocator = std::allocator<Key> >
  class Set>
static void failed_insertion_with_hint()
{
  {
    Set<int> src;
    Set<int> dst;
    src.emplace(10);
    src.emplace(20);
    dst.emplace(10);
    dst.emplace(20);

    typename Set<int>::node_type nh = src.extract(10);

    BOOST_TEST(dst.insert(dst.find(10), boost::move(nh)) == dst.find(10));
    BOOST_TEST(nh);
    BOOST_TEST(!nh.empty());
    BOOST_TEST(nh.value() == 10);

    BOOST_TEST(dst.insert(dst.find(20), boost::move(nh)) == dst.find(10));
    BOOST_TEST(nh);
    BOOST_TEST(!nh.empty());
    BOOST_TEST(nh.value() == 10);

    BOOST_TEST(src.count(10) == 0);
    BOOST_TEST(src.count(20) == 1);
    BOOST_TEST(dst.count(10) == 1);
    BOOST_TEST(dst.count(20) == 1);
  }

  {
    Map<int, int> src;
    Map<int, int> dst;
    src.emplace(10, 30);
    src.emplace(20, 5);
    dst.emplace(10, 20);
    dst.emplace(20, 2);

    typename Map<int, int>::node_type nh = src.extract(10);
    BOOST_TEST(dst.insert(dst.find(10), boost::move(nh)) == dst.find(10));
    BOOST_TEST(nh);
    BOOST_TEST(!nh.empty());
    BOOST_TEST(nh.key() == 10);
    BOOST_TEST(nh.mapped() == 30);
    BOOST_TEST(dst[10] == 20);

    BOOST_TEST(dst.insert(dst.find(20), boost::move(nh)) == dst.find(10));
    BOOST_TEST(nh);
    BOOST_TEST(!nh.empty());
    BOOST_TEST(nh.key() == 10);
    BOOST_TEST(nh.mapped() == 30);
    BOOST_TEST(dst[10] == 20);

    BOOST_TEST(src.count(10) == 0);
    BOOST_TEST(src.count(20) == 1);
    BOOST_TEST(dst.count(10) == 1);
    BOOST_TEST(dst.count(20) == 1);
  }
}

template <typename NodeHandle>
bool node_handle_compare(
  NodeHandle const& nh, typename NodeHandle::value_type const& x)
{
  return x == nh.value();
}

template <typename NodeHandle>
bool node_handle_compare(
  NodeHandle const& nh, std::pair<typename NodeHandle::key_type const,
                          typename NodeHandle::mapped_type> const& x)
{
  return x.first == nh.key() && x.second == nh.mapped();
}

template <typename Container> void node_handle_tests_impl(Container& c)
{
  typedef typename Container::node_type node_type;

  typename Container::value_type value = *c.begin();

  node_type n1;
  BOOST_TEST(!n1);
  BOOST_TEST(n1.empty());

  node_type n2 = c.extract(c.begin());
  BOOST_TEST(n2);
  BOOST_TEST(!n2.empty());
  node_handle_compare(n2, value);

  node_type n3 = boost::move(n2);
  BOOST_TEST(n3);
  BOOST_TEST(!n2);
  node_handle_compare(n3, value);
  // TODO: Check that n2 doesn't have an allocator?
  //       Maybe by swapping and observing that the allocator is
  //       swapped rather than moved?

  n1 = boost::move(n3);
  BOOST_TEST(n1);
  BOOST_TEST(!n3);
  node_handle_compare(n1, value);

  // Self move-assignment empties the node_handle.
  n1 = boost::move(n1);
  BOOST_TEST(!n1);

  n3 = boost::move(n3);
  BOOST_TEST(!n3);

  typename Container::value_type value1 = *c.begin();
  n1 = c.extract(c.begin());
  typename Container::value_type value2 = *c.begin();
  n2 = c.extract(c.begin());
  n3 = node_type();

  node_handle_compare(n1, value1);
  node_handle_compare(n2, value2);
  n1.swap(n2);
  BOOST_TEST(n1);
  BOOST_TEST(n2);
  node_handle_compare(n1, value2);
  node_handle_compare(n2, value1);

  BOOST_TEST(n1);
  BOOST_TEST(!n3);
  n1.swap(n3);
  BOOST_TEST(!n1);
  BOOST_TEST(n3);
  node_handle_compare(n3, value2);

  BOOST_TEST(!n1);
  BOOST_TEST(n2);
  n1.swap(n2);
  BOOST_TEST(n1);
  BOOST_TEST(!n2);
  node_handle_compare(n1, value1);

  node_type n4;
  BOOST_TEST(!n2);
  BOOST_TEST(!n4);
  n2.swap(n4);
  BOOST_TEST(!n2);
  BOOST_TEST(!n4);
}

template <template <class Key, class T, class Hash = boost::hash<Key>,
            class Pred = std::equal_to<Key>,
            class Allocator = std::allocator<std::pair<Key const, T> > >
          class Map,
  template <class Key, class Hash = boost::hash<Key>,
    class Pred = std::equal_to<Key>, class Allocator = std::allocator<Key> >
  class Set>
static void node_handle_tests()
{
  Set<int> x1;
  x1.emplace(100);
  x1.emplace(140);
  x1.emplace(-55);
  node_handle_tests_impl(x1);

  Map<int, std::string> x2;
  x2.emplace(10, "ten");
  x2.emplace(-23, "twenty");
  x2.emplace(-76, "thirty");
  node_handle_tests_impl(x2);
}

#ifdef BOOST_UNORDERED_FOA_TESTS

template <class X> typename X::iterator insert_empty_node(X& x)
{
  typedef typename X::node_type node_type;
  return x.insert(node_type()).position;
}

#else

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
typename boost::unordered_map<Key, T, Hash, KeyEqual, Allocator>::iterator
insert_empty_node(boost::unordered_map<Key, T, Hash, KeyEqual, Allocator>& c)
{
  typedef
    typename boost::unordered_map<Key, T, Hash, KeyEqual, Allocator>::node_type
      node_type;

  return c.insert(node_type()).position;
}

template <class T, class Hash, class KeyEqual, class Allocator>
typename boost::unordered_set<T, Hash, KeyEqual, Allocator>::iterator
insert_empty_node(boost::unordered_set<T, Hash, KeyEqual, Allocator>& c)
{
  typedef typename boost::unordered_set<T, Hash, KeyEqual, Allocator>::node_type
    node_type;

  return c.insert(node_type()).position;
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
typename boost::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>::iterator
insert_empty_node(
  boost::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>& c)
{
  typedef typename boost::unordered_multimap<Key, T, Hash, KeyEqual,
    Allocator>::node_type node_type;

  return c.insert(node_type());
}

template <class T, class Hash, class KeyEqual, class Allocator>
typename boost::unordered_multiset<T, Hash, KeyEqual, Allocator>::iterator
insert_empty_node(boost::unordered_multiset<T, Hash, KeyEqual, Allocator>& c)
{
  typedef
    typename boost::unordered_multiset<T, Hash, KeyEqual, Allocator>::node_type
      node_type;

  return c.insert(node_type());
}

#endif

template <typename Container1, typename Container2>
static void insert_node_handle_unique(Container1& c1, Container2& c2)
{
  typedef typename Container1::node_type node_type;
  typedef typename Container1::value_type value_type;
  BOOST_STATIC_ASSERT(
    (boost::is_same<node_type, typename Container2::node_type>::value));

  typedef typename Container1::iterator iterator1;
  typedef typename Container2::iterator iterator2;
  typedef typename Container2::insert_return_type insert_return_type2;

  Container1 c1_copy(c1);
  Container2 c2_copy;

  iterator1 r1 = insert_empty_node(c1);
  insert_return_type2 r2 = c2.insert(node_type());
  BOOST_TEST(r1 == c1.end());
  BOOST_TEST(!r2.inserted);
  BOOST_TEST(!r2.node);
  BOOST_TEST(r2.position == c2.end());

  while (!c1.empty()) {
    value_type v = *c1.begin();
    value_type const* v_ptr = boost::to_address(c1.begin());
    std::size_t count = c2.count(test::get_key<Container1>(v));
    insert_return_type2 r = c2.insert(c1.extract(c1.begin()));
    if (!count) {
      BOOST_TEST(r.inserted);
      BOOST_TEST_EQ(c2.count(test::get_key<Container1>(v)), count + 1);
      BOOST_TEST(r.position != c2.end());
      BOOST_TEST(boost::to_address(r.position) == v_ptr);
      BOOST_TEST(!r.node);
    } else {
      BOOST_TEST(!r.inserted);
      BOOST_TEST_EQ(c2.count(test::get_key<Container1>(v)), count);
      BOOST_TEST(r.position != c2.end());
      BOOST_TEST(
        test::get_key<Container2>(*r.position) == test::get_key<Container2>(v));
      BOOST_TEST(r.node);
      node_handle_compare(r.node, v);
    }
  }

  while (!c1_copy.empty()) {
    value_type v = *c1_copy.begin();
    value_type const* v_ptr = boost::to_address(c1_copy.begin());
    std::size_t count = c2_copy.count(test::get_key<Container1>(v));
    iterator2 pos =
      c2_copy.insert(c2_copy.begin(), c1_copy.extract(c1_copy.begin()));
    if (!count) {
      BOOST_TEST_EQ(c2_copy.count(test::get_key<Container1>(v)), count + 1);
      BOOST_TEST(pos != c2.end());
      BOOST_TEST(boost::to_address(pos) == v_ptr);
    } else {
      BOOST_TEST_EQ(c2_copy.count(test::get_key<Container1>(v)), count);
      BOOST_TEST(pos != c2_copy.end());
      BOOST_TEST(
        test::get_key<Container2>(*pos) == test::get_key<Container2>(v));
    }
  }
}

template <typename Container1, typename Container2>
static void insert_node_handle_unique2(Container1& c1, Container2& c2)
{
  typedef typename Container1::node_type node_type;
  typedef typename Container1::value_type value_type;
  BOOST_STATIC_ASSERT(
    (boost::is_same<node_type, typename Container2::node_type>::value));

  // typedef typename Container1::insert_return_type
  // insert_return_type1;
  typedef typename Container2::insert_return_type insert_return_type2;

  while (!c1.empty()) {
    value_type v = *c1.begin();
    value_type const* v_ptr = boost::to_address(c1.begin());
    std::size_t count = c2.count(test::get_key<Container1>(v));
    insert_return_type2 r = c2.insert(c1.extract(test::get_key<Container1>(v)));
    if (r.inserted) {
      BOOST_TEST_EQ(c2.count(test::get_key<Container1>(v)), count + 1);
      BOOST_TEST(r.position != c2.end());
      BOOST_TEST(boost::to_address(r.position) == v_ptr);
      BOOST_TEST(!r.node);
    } else {
      BOOST_TEST_EQ(c2.count(test::get_key<Container1>(v)), count);
      BOOST_TEST(r.position != c2.end());
      BOOST_TEST(
        test::get_key<Container2>(*r.position) == test::get_key<Container2>(v));
      BOOST_TEST(r.node);
      node_handle_compare(r.node, v);
    }
  }
}

struct hash_thing
{
  std::size_t operator()(int x) const
  {
    return static_cast<std::size_t>(x * 13 + 5);
  }
};

#ifdef BOOST_UNORDERED_FOA_TESTS

UNORDERED_AUTO_TEST (examples) {
  example1<boost::unordered_node_map>();
  example2<boost::unordered_node_set>();
  example3<boost::unordered_node_set>();
  failed_insertion_with_hint<boost::unordered_node_map,
    boost::unordered_node_set>();
  node_handle_tests<boost::unordered_node_map, boost::unordered_node_set>();
}

UNORDERED_AUTO_TEST (insert_node_handle_unique_tests) {
  {
    boost::unordered_node_set<int> x1;
    boost::unordered_node_set<int> x2;
    x1.emplace(100);
    x1.emplace(140);
    x1.emplace(-55);
    x2.emplace(140);
    insert_node_handle_unique(x1, x2);
    BOOST_TEST(x2.size() == 3);
  }

  {
    boost::unordered_node_set<int> x1;
    boost::unordered_node_set<int> x2;
    x1.emplace(100);
    x1.emplace(140);
    x1.emplace(-55);
    x2.emplace(140);
    insert_node_handle_unique(x1, x2);
    BOOST_TEST(x2.size() == 3);
  }

  {
    boost::unordered_node_map<int, int, hash_thing> x1;
    boost::unordered_node_map<int, int> x2;
    x1.emplace(67, 50);
    x1.emplace(23, 45);
    x1.emplace(18, 19);
    x2.emplace(23, 50);
    x2.emplace(12, 49);
    insert_node_handle_unique(x1, x2);
    BOOST_TEST(x2.size() == 4);
  }

  {
    boost::unordered_node_map<int, int, hash_thing> x1;
    boost::unordered_node_map<int, int> x2;
    x1.emplace(67, 50);
    x1.emplace(23, 45);
    x1.emplace(18, 19);
    x2.emplace(23, 50);
    x2.emplace(12, 49);
    insert_node_handle_unique(x1, x2);
    BOOST_TEST(x2.size() == 4);
  }
}

UNORDERED_AUTO_TEST (insert_node_handle_unique_tests2) {
  {
    boost::unordered_node_set<int> x1;
    boost::unordered_node_set<int> x2;
    x1.emplace(100);
    x1.emplace(140);
    x1.emplace(-55);
    x2.emplace(140);
    insert_node_handle_unique2(x1, x2);
    BOOST_TEST(x2.size() == 3);
  }

  {
    boost::unordered_node_set<int> x1;
    boost::unordered_node_set<int> x2;
    x1.emplace(100);
    x1.emplace(140);
    x1.emplace(-55);
    x2.emplace(140);
    insert_node_handle_unique2(x1, x2);
    BOOST_TEST(x2.size() == 3);
  }

  {
    boost::unordered_node_map<int, int, hash_thing> x1;
    boost::unordered_node_map<int, int> x2;
    x1.emplace(67, 50);
    x1.emplace(23, 45);
    x1.emplace(18, 19);
    x2.emplace(23, 50);
    x2.emplace(12, 49);
    insert_node_handle_unique2(x1, x2);
    BOOST_TEST(x2.size() == 4);
  }

  {
    boost::unordered_node_map<int, int, hash_thing> x1;
    boost::unordered_node_map<int, int> x2;
    x1.emplace(67, 50);
    x1.emplace(23, 45);
    x1.emplace(18, 19);
    x2.emplace(23, 50);
    x2.emplace(12, 49);
    insert_node_handle_unique2(x1, x2);
    BOOST_TEST(x2.size() == 4);
  }
}

#else
UNORDERED_AUTO_TEST (examples) {
  example1<boost::unordered_map>();
  example2<boost::unordered_set>();
  example3<boost::unordered_set>();
  failed_insertion_with_hint<boost::unordered_map, boost::unordered_set>();
  node_handle_tests<boost::unordered_map, boost::unordered_set>();
}

template <typename Container1, typename Container2>
void insert_node_handle_equiv(Container1& c1, Container2& c2)
{
  typedef typename Container1::node_type node_type;
  typedef typename Container1::value_type value_type;
  BOOST_STATIC_ASSERT(
    (boost::is_same<node_type, typename Container2::node_type>::value));

  typedef typename Container1::iterator iterator1;
  typedef typename Container2::iterator iterator2;

  iterator1 r1 = insert_empty_node(c1);
  iterator2 r2 = c2.insert(node_type());
  BOOST_TEST(r1 == c1.end());
  BOOST_TEST(r2 == c2.end());

  while (!c1.empty()) {
    value_type v = *c1.begin();
    value_type const* v_ptr = boost::to_address(c1.begin());
    std::size_t count = c2.count(test::get_key<Container1>(v));
    iterator2 r = c2.insert(c1.extract(c1.begin()));
    BOOST_TEST_EQ(c2.count(test::get_key<Container1>(v)), count + 1);
    BOOST_TEST(r != c2.end());
    BOOST_TEST(boost::to_address(r) == v_ptr);
  }
}

UNORDERED_AUTO_TEST (insert_node_handle_unique_tests) {
  {
    boost::unordered_set<int> x1;
    boost::unordered_set<int> x2;
    x1.emplace(100);
    x1.emplace(140);
    x1.emplace(-55);
    x2.emplace(140);
    insert_node_handle_unique(x1, x2);
    BOOST_TEST(x2.size() == 3);
  }

  {
    boost::unordered_multiset<int> x1;
    boost::unordered_set<int> x2;
    x1.emplace(100);
    x1.emplace(140);
    x1.emplace(-55);
    x2.emplace(140);
    insert_node_handle_unique(x1, x2);
    BOOST_TEST(x2.size() == 3);
  }

  {
    boost::unordered_map<int, int, hash_thing> x1;
    boost::unordered_map<int, int> x2;
    x1.emplace(67, 50);
    x1.emplace(23, 45);
    x1.emplace(18, 19);
    x2.emplace(23, 50);
    x2.emplace(12, 49);
    insert_node_handle_unique(x1, x2);
    BOOST_TEST(x2.size() == 4);
  }

  {
    boost::unordered_multimap<int, int, hash_thing> x1;
    boost::unordered_map<int, int> x2;
    x1.emplace(67, 50);
    x1.emplace(23, 45);
    x1.emplace(18, 19);
    x2.emplace(23, 50);
    x2.emplace(12, 49);
    insert_node_handle_unique(x1, x2);
    BOOST_TEST(x2.size() == 4);
  }
}

UNORDERED_AUTO_TEST (insert_node_handle_equiv_tests) {
  {
    boost::unordered_multimap<int, int, hash_thing> x1;
    boost::unordered_multimap<int, int> x2;
    x1.emplace(67, 50);
    x1.emplace(67, 100);
    x1.emplace(23, 45);
    x1.emplace(18, 19);
    x2.emplace(23, 50);
    x2.emplace(12, 49);
    insert_node_handle_equiv(x1, x2);
    BOOST_TEST(x2.size() == 6);
  }

  {
    boost::unordered_map<int, int, hash_thing> x1;
    boost::unordered_multimap<int, int> x2;
    x1.emplace(67, 50);
    x1.emplace(67, 100);
    x1.emplace(23, 45);
    x1.emplace(18, 19);
    x2.emplace(23, 50);
    x2.emplace(12, 49);
    insert_node_handle_equiv(x1, x2);
    BOOST_TEST(x2.size() == 5);
  }

  {
    boost::unordered_multiset<int, hash_thing> x1;
    boost::unordered_multiset<int> x2;
    x1.emplace(67);
    x1.emplace(67);
    x1.emplace(23);
    x1.emplace(18);
    x2.emplace(23);
    x2.emplace(12);
    insert_node_handle_equiv(x1, x2);
    BOOST_TEST(x2.size() == 6);
  }

  {
    boost::unordered_set<int, hash_thing> x1;
    boost::unordered_multiset<int> x2;
    x1.emplace(67);
    x1.emplace(67);
    x1.emplace(23);
    x1.emplace(18);
    x2.emplace(23);
    x2.emplace(12);
    insert_node_handle_equiv(x1, x2);
    BOOST_TEST(x2.size() == 5);
  }
}

UNORDERED_AUTO_TEST (insert_node_handle_unique_tests2) {
  {
    boost::unordered_set<int> x1;
    boost::unordered_set<int> x2;
    x1.emplace(100);
    x1.emplace(140);
    x1.emplace(-55);
    x2.emplace(140);
    insert_node_handle_unique2(x1, x2);
    BOOST_TEST(x2.size() == 3);
  }

  {
    boost::unordered_multiset<int> x1;
    boost::unordered_set<int> x2;
    x1.emplace(100);
    x1.emplace(140);
    x1.emplace(-55);
    x2.emplace(140);
    insert_node_handle_unique2(x1, x2);
    BOOST_TEST(x2.size() == 3);
  }

  {
    boost::unordered_map<int, int, hash_thing> x1;
    boost::unordered_map<int, int> x2;
    x1.emplace(67, 50);
    x1.emplace(23, 45);
    x1.emplace(18, 19);
    x2.emplace(23, 50);
    x2.emplace(12, 49);
    insert_node_handle_unique2(x1, x2);
    BOOST_TEST(x2.size() == 4);
  }

  {
    boost::unordered_multimap<int, int, hash_thing> x1;
    boost::unordered_map<int, int> x2;
    x1.emplace(67, 50);
    x1.emplace(23, 45);
    x1.emplace(18, 19);
    x2.emplace(23, 50);
    x2.emplace(12, 49);
    insert_node_handle_unique2(x1, x2);
    BOOST_TEST(x2.size() == 4);
  }
}

#endif

RUN_TESTS()
