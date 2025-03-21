
// Copyright 2006-2009 Daniel James.
// Copyright 2022 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_UNORDERED_TEST_OBJECTS_HEADER)
#define BOOST_UNORDERED_TEST_OBJECTS_HEADER

#include "../helpers/count.hpp"
#include "../helpers/fwd.hpp"
#include "../helpers/memory.hpp"
#include <boost/config.hpp>
#include <boost/limits.hpp>
#include <cstddef>

namespace test {
  // Note that the default hash function will work for any equal_to (but not
  // very well).
  class object;
  class movable;
  class implicitly_convertible;
  class hash;
  class less;
  class equal_to;
  template <class T> class allocator1;
  template <class T> class allocator2;
  object generate(object const*, random_generator);
  movable generate(movable const*, random_generator);
  implicitly_convertible generate(
    implicitly_convertible const*, random_generator);

  inline void ignore_variable(void const*) {}

  class object : private counted_object
  {
    friend class hash;
    friend class equal_to;
    friend class less;
    int tag1_, tag2_;

  public:
    explicit object(int t1 = 0, int t2 = 0) : tag1_(t1), tag2_(t2) {}

    ~object()
    {
      tag1_ = -1;
      tag2_ = -1;
    }

    friend bool operator==(object const& x1, object const& x2)
    {
      return x1.tag1_ == x2.tag1_ && x1.tag2_ == x2.tag2_;
    }

    friend bool operator!=(object const& x1, object const& x2)
    {
      return x1.tag1_ != x2.tag1_ || x1.tag2_ != x2.tag2_;
    }

    friend bool operator<(object const& x1, object const& x2)
    {
      return x1.tag1_ < x2.tag1_ ||
             (x1.tag1_ == x2.tag1_ && x1.tag2_ < x2.tag2_);
    }

    friend object generate(object const*, random_generator g)
    {
      int* x = 0;
      return object(generate(x, g), generate(x, g));
    }

    friend std::ostream& operator<<(std::ostream& out, object const& o)
    {
      return out << "(" << o.tag1_ << "," << o.tag2_ << ")";
    }
  };

  class movable : private counted_object
  {
    friend class hash;
    friend class equal_to;
    friend class less;
    int tag1_, tag2_;

    BOOST_COPYABLE_AND_MOVABLE(movable)
  public:
    explicit movable(int t1 = 0, int t2 = 0) : tag1_(t1), tag2_(t2) {}

    movable(movable const& x)
        : counted_object(x), tag1_(x.tag1_), tag2_(x.tag2_)
    {
      BOOST_TEST(x.tag1_ != -1);
    }

    movable(BOOST_RV_REF(movable) x)
        : counted_object(x), tag1_(x.tag1_), tag2_(x.tag2_)
    {
      BOOST_TEST(x.tag1_ != -1);
      x.tag1_ = -1;
      x.tag2_ = -1;
    }

    movable& operator=(BOOST_COPY_ASSIGN_REF(movable) x) // Copy assignment
    {
      BOOST_TEST(x.tag1_ != -1);
      tag1_ = x.tag1_;
      tag2_ = x.tag2_;
      return *this;
    }

    movable& operator=(BOOST_RV_REF(movable) x) // Move assignment
    {
      BOOST_TEST(x.tag1_ != -1);
      tag1_ = x.tag1_;
      tag2_ = x.tag2_;
      x.tag1_ = -1;
      x.tag2_ = -1;
      return *this;
    }

    ~movable()
    {
      tag1_ = -1;
      tag2_ = -1;
    }

    friend bool operator==(movable const& x1, movable const& x2)
    {
      BOOST_TEST(x1.tag1_ != -1 && x2.tag1_ != -1);
      return x1.tag1_ == x2.tag1_ && x1.tag2_ == x2.tag2_;
    }

    friend bool operator!=(movable const& x1, movable const& x2)
    {
      BOOST_TEST(x1.tag1_ != -1 && x2.tag1_ != -1);
      return x1.tag1_ != x2.tag1_ || x1.tag2_ != x2.tag2_;
    }

    friend bool operator<(movable const& x1, movable const& x2)
    {
      BOOST_TEST(x1.tag1_ != -1 && x2.tag1_ != -1);
      return x1.tag1_ < x2.tag1_ ||
             (x1.tag1_ == x2.tag1_ && x1.tag2_ < x2.tag2_);
    }

    friend movable generate(movable const*, random_generator g)
    {
      int* x = 0;
      return movable(generate(x, g), generate(x, g));
    }

    friend std::ostream& operator<<(std::ostream& out, movable const& o)
    {
      return out << "(" << o.tag1_ << "," << o.tag2_ << ")";
    }
  };

  class implicitly_convertible : private counted_object
  {
    int tag1_, tag2_;

  public:
    explicit implicitly_convertible(int t1 = 0, int t2 = 0)
        : tag1_(t1), tag2_(t2)
    {
    }

    operator object() const { return object(tag1_, tag2_); }

    operator movable() const { return movable(tag1_, tag2_); }

    friend implicitly_convertible generate(
      implicitly_convertible const*, random_generator g)
    {
      int* x = 0;
      return implicitly_convertible(generate(x, g), generate(x, g));
    }

    friend std::ostream& operator<<(
      std::ostream& out, implicitly_convertible const& o)
    {
      return out << "(" << o.tag1_ << "," << o.tag2_ << ")";
    }
  };

  // Note: This is a deliberately bad hash function.
  class hash BOOST_FINAL
  {
    int type_;

  public:
    hash() : type_(0) {}

    explicit hash(int t) : type_(t) {}

    std::size_t operator()(object const& x) const
    {
      unsigned result;
      switch (type_) {
      case 1:
        result = static_cast<unsigned>(x.tag1_);
        break;
      case 2:
        result = static_cast<unsigned>(x.tag2_);
        break;
      default:
        result =
          static_cast<unsigned>(x.tag1_) + static_cast<unsigned>(x.tag2_);
      }
      return result;
    }

    std::size_t operator()(movable const& x) const
    {
      unsigned result;
      switch (type_) {
      case 1:
        result = static_cast<unsigned>(x.tag1_);
        break;
      case 2:
        result = static_cast<unsigned>(x.tag2_);
        break;
      default:
        result =
          static_cast<unsigned>(x.tag1_) + static_cast<unsigned>(x.tag2_);
      }
      return result;
    }

    std::size_t operator()(int x) const
    {
      unsigned result;
      switch (type_) {
      case 1:
        result = static_cast<unsigned>(x);
        break;
      case 2:
        result = static_cast<unsigned>(x) * 7;
        break;
      default:
        result = static_cast<unsigned>(x) * 256;
      }
      return result;
    }

    friend bool operator==(hash const& x1, hash const& x2)
    {
      return x1.type_ == x2.type_;
    }

    friend bool operator!=(hash const& x1, hash const& x2)
    {
      return x1.type_ != x2.type_;
    }
  };

  std::size_t hash_value(test::object const& x) { return hash()(x); }

  std::size_t hash_value(test::movable const& x) { return hash()(x); }

  class less
  {
    int type_;

  public:
    explicit less(int t = 0) : type_(t) {}

    bool operator()(object const& x1, object const& x2) const
    {
      switch (type_) {
      case 1:
        return x1.tag1_ < x2.tag1_;
      case 2:
        return x1.tag2_ < x2.tag2_;
      default:
        return x1 < x2;
      }
    }

    bool operator()(movable const& x1, movable const& x2) const
    {
      switch (type_) {
      case 1:
        return x1.tag1_ < x2.tag1_;
      case 2:
        return x1.tag2_ < x2.tag2_;
      default:
        return x1 < x2;
      }
    }

    bool operator()(int x1, int x2) const { return x1 < x2; }

    friend bool operator==(less const& x1, less const& x2)
    {
      return x1.type_ == x2.type_;
    }
  };

  class equal_to BOOST_FINAL
  {
    int type_;

  public:
    equal_to() : type_(0) {}

    explicit equal_to(int t) : type_(t) {}

    bool operator()(object const& x1, object const& x2) const
    {
      switch (type_) {
      case 1:
        return x1.tag1_ == x2.tag1_;
      case 2:
        return x1.tag2_ == x2.tag2_;
      default:
        return x1 == x2;
      }
    }

    bool operator()(movable const& x1, movable const& x2) const
    {
      switch (type_) {
      case 1:
        return x1.tag1_ == x2.tag1_;
      case 2:
        return x1.tag2_ == x2.tag2_;
      default:
        return x1 == x2;
      }
    }

    bool operator()(int x1, int x2) const { return x1 == x2; }

    friend bool operator==(equal_to const& x1, equal_to const& x2)
    {
      return x1.type_ == x2.type_;
    }

    friend bool operator!=(equal_to const& x1, equal_to const& x2)
    {
      return x1.type_ != x2.type_;
    }

    friend less create_compare(equal_to x) { return less(x.type_); }
  };

  // allocator1 only has the old fashioned 'construct' method and has
  // a few less typedefs. allocator2 uses a custom pointer class.

  template <class T> class allocator1
  {
  public:
    int tag_;

    typedef T value_type;

    template <class U> struct rebind
    {
      typedef allocator1<U> other;
    };

    allocator1() : tag_(0) { detail::tracker.allocator_ref(); }

    explicit allocator1(int t) : tag_(t) { detail::tracker.allocator_ref(); }

    template <class Y> allocator1(allocator1<Y> const& x) : tag_(x.tag_)
    {
      detail::tracker.allocator_ref();
    }

    allocator1(allocator1 const& x) : tag_(x.tag_)
    {
      detail::tracker.allocator_ref();
    }

    ~allocator1() { detail::tracker.allocator_unref(); }

    T* allocate(std::size_t n)
    {
      T* ptr(static_cast<T*>(::operator new(n * sizeof(T))));
      detail::tracker.track_allocate((void*)ptr, n, sizeof(T), tag_);
      return ptr;
    }

    T* allocate(std::size_t n, void const*)
    {
      T* ptr(static_cast<T*>(::operator new(n * sizeof(T))));
      detail::tracker.track_allocate((void*)ptr, n, sizeof(T), tag_);
      return ptr;
    }

    void deallocate(T* p, std::size_t n)
    {
      detail::tracker.track_deallocate((void*)p, n, sizeof(T), tag_);
      ::operator delete((void*)p);
    }

#if BOOST_UNORDERED_CXX11_CONSTRUCTION
    template <typename U, typename... Args> void construct(U* p, Args&&... args)
    {
      detail::tracker.track_construct((void*)p, sizeof(U), tag_);
      new (p) U(boost::forward<Args>(args)...);
    }

    template <typename U> void destroy(U* p)
    {
      detail::tracker.track_destroy((void*)p, sizeof(U), tag_);
      p->~U();

      // Work around MSVC buggy unused parameter warning.
      ignore_variable(&p);
    }
#else
  private:
    // I'm going to claim in the documentation that construct/destroy
    // is never used when C++11 support isn't available, so might as
    // well check that in the text.
    // TODO: Or maybe just disallow them for values?
    template <typename U> void construct(U* p);
    template <typename U, typename A0> void construct(U* p, A0 const&);
    template <typename U, typename A0, typename A1>
    void construct(U* p, A0 const&, A1 const&);
    template <typename U, typename A0, typename A1, typename A2>
    void construct(U* p, A0 const&, A1 const&, A2 const&);
    template <typename U> void destroy(U* p);

  public:
#endif

    bool operator==(allocator1 const& x) const { return tag_ == x.tag_; }

    bool operator!=(allocator1 const& x) const { return tag_ != x.tag_; }

    enum
    {
      is_select_on_copy = false,
      is_propagate_on_swap = false,
      is_propagate_on_assign = false,
      is_propagate_on_move = false
    };
  };

  template <class T> class ptr;
  template <class T> class const_ptr;

  struct void_ptr
  {
#if !defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
    template <typename T> friend class ptr;

  private:
#endif

    void* ptr_;

  public:
    void_ptr() : ptr_(0) {}

    template <typename T> explicit void_ptr(ptr<T> const& x) : ptr_(x.ptr_) {}

    // I'm not using the safe bool idiom because the containers should be
    // able to cope with bool conversions.
    operator bool() const { return !!ptr_; }

    bool operator==(void_ptr const& x) const { return ptr_ == x.ptr_; }
    bool operator!=(void_ptr const& x) const { return ptr_ != x.ptr_; }
  };

  class void_const_ptr
  {
#if !defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
    template <typename T> friend class const_ptr;

  private:
#endif

    void* ptr_;

  public:
    void_const_ptr() : ptr_(0) {}

    template <typename T>
    explicit void_const_ptr(const_ptr<T> const& x) : ptr_(x.ptr_)
    {
    }

    // I'm not using the safe bool idiom because the containers should be
    // able to cope with bool conversions.
    operator bool() const { return !!ptr_; }

    bool operator==(void_const_ptr const& x) const { return ptr_ == x.ptr_; }
    bool operator!=(void_const_ptr const& x) const { return ptr_ != x.ptr_; }
  };

  template <class T> class ptr
  {
    friend class allocator2<T>;
    friend class const_ptr<T>;
    friend struct void_ptr;

  public:
    T* ptr_;
    ptr(T* x) : ptr_(x) {}
    ptr() : ptr_(0) {}
    explicit ptr(void_ptr const& x) : ptr_((T*)x.ptr_) {}

    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    ptr& operator++()
    {
      ++ptr_;
      return *this;
    }
    ptr operator++(int)
    {
      ptr tmp(*this);
      ++ptr_;
      return tmp;
    }

    ptr operator+(std::ptrdiff_t s) const { return ptr<T>(ptr_ + s); }
    friend ptr operator+(std::ptrdiff_t s, ptr p) { return ptr<T>(s + p.ptr_); }

    std::ptrdiff_t operator-(ptr p) const { return ptr_ - p.ptr_; }
    ptr operator-(std::ptrdiff_t s) const { return ptr(ptr_ - s); }

    ptr& operator+=(std::ptrdiff_t s) { ptr_ += s; return *this; }

    T& operator[](std::ptrdiff_t s) const { return ptr_[s]; }
    bool operator!() const { return !ptr_; }

    static ptr pointer_to(T& p) {
      return ptr(&p);
    }

    // I'm not using the safe bool idiom because the containers should be
    // able to cope with bool conversions.
    operator bool() const { return !!ptr_; }

    bool operator==(ptr const& x) const { return ptr_ == x.ptr_; }
    bool operator!=(ptr const& x) const { return ptr_ != x.ptr_; }
    bool operator<(ptr const& x) const { return ptr_ < x.ptr_; }
    bool operator>(ptr const& x) const { return ptr_ > x.ptr_; }
    bool operator<=(ptr const& x) const { return ptr_ <= x.ptr_; }
    bool operator>=(ptr const& x) const { return ptr_ >= x.ptr_; }
  };

  template <class T> class const_ptr
  {
    friend class allocator2<T>;
    friend struct const_void_ptr;

    T const* ptr_;

    const_ptr(T const* ptr) : ptr_(ptr) {}

  public:
    const_ptr() : ptr_(0) {}
    const_ptr(ptr<T> const& x) : ptr_(x.ptr_) {}
    explicit const_ptr(void_const_ptr const& x) : ptr_((T const*)x.ptr_) {}

    T const& operator*() const { return *ptr_; }
    T const* operator->() const { return ptr_; }
    const_ptr& operator++()
    {
      ++ptr_;
      return *this;
    }
    const_ptr operator++(int)
    {
      const_ptr tmp(*this);
      ++ptr_;
      return tmp;
    }
    const_ptr operator+(std::ptrdiff_t s) const { return const_ptr(ptr_ + s); }
    friend const_ptr operator+(std::ptrdiff_t s, const_ptr p)
    {
      return ptr<T>(s + p.ptr_);
    }
    T const& operator[](int s) const { return ptr_[s]; }
    bool operator!() const { return !ptr_; }
    operator bool() const { return !!ptr_; }

    bool operator==(const_ptr const& x) const { return ptr_ == x.ptr_; }
    bool operator!=(const_ptr const& x) const { return ptr_ != x.ptr_; }
    bool operator<(const_ptr const& x) const { return ptr_ < x.ptr_; }
    bool operator>(const_ptr const& x) const { return ptr_ > x.ptr_; }
    bool operator<=(const_ptr const& x) const { return ptr_ <= x.ptr_; }
    bool operator>=(const_ptr const& x) const { return ptr_ >= x.ptr_; }
  };

  template <class T> class allocator2
  {
#ifdef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
  public:
#else
    template <class> friend class allocator2;
#endif
    int tag_;

  public:
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef void_ptr void_pointer;
    typedef void_const_ptr const_void_pointer;
    typedef ptr<T> pointer;
    typedef const_ptr<T> const_pointer;
    typedef T& reference;
    typedef T const& const_reference;
    typedef T value_type;

    template <class U> struct rebind
    {
      typedef allocator2<U> other;
    };

    allocator2() : tag_(0) { detail::tracker.allocator_ref(); }

    explicit allocator2(int t) : tag_(t) { detail::tracker.allocator_ref(); }

    template <class Y> allocator2(allocator2<Y> const& x) : tag_(x.tag_)
    {
      detail::tracker.allocator_ref();
    }

    allocator2(allocator2 const& x) : tag_(x.tag_)
    {
      detail::tracker.allocator_ref();
    }

    ~allocator2() { detail::tracker.allocator_unref(); }

    pointer address(reference r) { return pointer(&r); }

    const_pointer address(const_reference r) { return const_pointer(&r); }

    pointer allocate(size_type n)
    {
      pointer p(static_cast<T*>(::operator new(n * sizeof(T))));
      detail::tracker.track_allocate((void*)p.ptr_, n, sizeof(T), tag_);
      return p;
    }

    pointer allocate(size_type n, void const*)
    {
      pointer ptr(static_cast<T*>(::operator new(n * sizeof(T))));
      detail::tracker.track_allocate((void*)ptr, n, sizeof(T), tag_);
      return ptr;
    }

    void deallocate(pointer p, size_type n)
    {
      detail::tracker.track_deallocate((void*)p.ptr_, n, sizeof(T), tag_);
      ::operator delete((void*)p.ptr_);
    }

#if defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)
    template <class U, class V> void construct(U* p, V const& v)
    {
      detail::tracker.track_construct((void*)p, sizeof(U), tag_);
      new (p) U(v);
    }
#else
    template <class U, class... Args>
    void construct(U* p, BOOST_FWD_REF(Args)... args)
    {
      detail::tracker.track_construct((void*)p, sizeof(U), tag_);
      new (p) U(boost::forward<Args>(args)...);
    }
#endif

    template <class U> void destroy(U* p)
    {
      detail::tracker.track_destroy((void*)p, sizeof(U), tag_);
      p->~U();
    }

    size_type max_size() const
    {
      return (std::numeric_limits<size_type>::max)();
    }

    bool operator==(allocator2 const& x) const { return tag_ == x.tag_; }

    bool operator!=(allocator2 const& x) const { return tag_ != x.tag_; }

    enum
    {
      is_select_on_copy = false,
      is_propagate_on_swap = false,
      is_propagate_on_assign = false,
      is_propagate_on_move = false
    };
  };

  template <class T>
  bool equivalent_impl(
    allocator1<T> const& x, allocator1<T> const& y, test::derived_type)
  {
    return x == y;
  }

  template <class T>
  bool equivalent_impl(
    allocator2<T> const& x, allocator2<T> const& y, test::derived_type)
  {
    return x == y;
  }
}

namespace boost {
  template <> struct pointer_traits< ::test::void_ptr>
  {
    template <class U> struct rebind_to
    {
      typedef ::test::ptr<U> type;
    };
  };
} // namespace boost

#endif
