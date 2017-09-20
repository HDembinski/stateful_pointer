#ifndef STATEFUL_POINTER_TAGGED_PTR_HPP
#define STATEFUL_POINTER_TAGGED_PTR_HPP

#include "boost/align/aligned_alloc.hpp"
#include "boost/align/alignment_of.hpp"
#include "boost/assert.hpp"
#include "boost/cstdint.hpp"
#include "boost/type_traits.hpp"
#include "cstddef"

namespace stateful_pointer {

namespace detail {
constexpr uintptr_t max(uintptr_t a, uintptr_t b) { return a > b ? a : b; }
constexpr unsigned pow2(unsigned n) noexcept {
  return n > 0 ? 2 * pow2(n - 1) : 1;
}
constexpr uintptr_t make_ptr_mask(unsigned n) noexcept {
  return ~uintptr_t(0) << n;
}
}

template <typename T, unsigned Nbits> class tagged_ptr {
public:
  using bits_type = uintptr_t;
  using pos_type = std::size_t; // only for array version
  using element_type = typename ::boost::remove_extent<T>::type;
  using pointer = element_type *;
  using reference = element_type &;

  constexpr tagged_ptr() noexcept : value(0) {}

  // tagged_ptr models exclusive ownership, no copies allowed
  tagged_ptr(const tagged_ptr &) = delete;
  tagged_ptr &operator=(const tagged_ptr &) = delete;

  tagged_ptr(tagged_ptr &&other) noexcept : value(other.value) {
    other.value = 0;
  }

  tagged_ptr &operator=(tagged_ptr &&other) noexcept {
    if (this != &other) {
      value = other.value;
      other.value = 0;
    }
    return *this;
  }

  /// move constructor that allows conversion between base and derived
  template <typename U, typename = typename ::boost::enable_if_c<
                            !(::boost::is_array<U>::value) &&
                            ::boost::is_convertible<U *, T *>::value>::type>
  tagged_ptr(tagged_ptr<U, Nbits> &&other) noexcept : value(other.value) {
    other.value = 0;
  }

  /// move assignment that allows conversion between base and derived
  template <typename U, typename = typename ::boost::enable_if_c<
                            !(::boost::is_array<U>::value) &&
                            ::boost::is_convertible<U *, T *>::value>::type>
  tagged_ptr &operator=(tagged_ptr<U, Nbits> &&other) noexcept {
    if (this != &other) {
      value = other.value;
      other.value = 0;
    }
    return *this;
  }

  ~tagged_ptr() {
    auto tp = get();
    if (tp) {
      if (::boost::is_array<T>::value) {
        auto iter = tp;
        auto end_p = array_end_p(tp);
        tp = reinterpret_cast<pointer>(end_p);
        if (!::boost::has_trivial_destructor<element_type>::value) {
          auto end = *end_p;
          while (iter != end)
            (iter++)->~element_type();
        }
      } else {
        // automatically skipped if T has trivial destructor
        tp->~element_type();
      }
      ::boost::alignment::aligned_free(tp);
    }
  }

  /// get tag bits as integral type
  bits_type bits() const noexcept { return value & tag_mask; }

  /// set tag bits via integral type, ptr bits are not overridden
  void bits(bits_type b) noexcept {
    value &= ptr_mask;       // clear old bits
    value |= (b & tag_mask); // set new bits
  }

  /// get bit at position pos
  bool bit(unsigned pos) const noexcept { return value & (1 << pos); }

  /// set bit at position pos to value b
  void bit(unsigned pos, bool b) noexcept {
    BOOST_ASSERT(pos < Nbits);
    if (b)
      value |= (1 << pos);
    else
      value &= ~(1 << pos);
  }

  /// get raw pointer in the fast way possible (no checks for nullness)
  pointer get() const noexcept { return extract_ptr(value); }

  /// release ownership of raw pointer, bits remain intact
  pointer release() noexcept {
    auto tmp = get();
    value &= ~ptr_mask; // nullify pointer bits
    return tmp;
  }

  /// reset ptr and bits to p
  void reset(tagged_ptr p = tagged_ptr()) noexcept { p.swap(*this); }

  void swap(tagged_ptr &other) noexcept { std::swap(value, other.value); }

  /// dereference operator, throws error in debug mode if pointer is null
  auto operator*() const -> reference {
    const auto p = get();
    BOOST_ASSERT(p != pointer());
    return *p;
  }

  /// array element access (only for array version)
  template <typename U = T,
            typename = typename ::boost::enable_if<::boost::is_array<U>>::type>
  reference operator[](pos_type i) const {
    auto p = get();
    BOOST_ASSERT((p + i) < *array_end_p(p));
    return *(p + i);
  }

  /// array size (only for array version)
  template <typename U = T,
            typename = typename ::boost::enable_if<::boost::is_array<U>>::type>
  pos_type size() const {
    const auto p = get();
    return *array_end_p(p) - p;
  }

  /// member access operator
  pointer operator->() const noexcept { return get(); }

  explicit operator bool() const noexcept { return static_cast<bool>(get()); }

  bool operator!() const noexcept { return get() == 0; }

private:
  static constexpr bits_type ptr_mask = detail::make_ptr_mask(Nbits);
  static constexpr bits_type tag_mask = ~ptr_mask;

  static pointer extract_ptr(bits_type v) noexcept {
    return reinterpret_cast<pointer>(v & ptr_mask);
  }

  static pointer *array_end_p(pointer p) noexcept {
    return reinterpret_cast<pointer *>(reinterpret_cast<char *>(p) -
                                       sizeof(pointer));
  };

  friend bool operator==(const tagged_ptr &a, const tagged_ptr &b) noexcept {
    return a.value == b.value;
  }

  friend bool operator!=(const tagged_ptr &a, const tagged_ptr &b) noexcept {
    return a.value != b.value;
  }

  friend bool operator<(const tagged_ptr &a, const tagged_ptr &b) noexcept {
    return a.value < b.value;
  }

  friend bool operator<=(const tagged_ptr &a, const tagged_ptr &b) noexcept {
    return a.value <= b.value;
  }

  friend bool operator>(const tagged_ptr &a, const tagged_ptr &b) noexcept {
    return a.value > b.value;
  }

  friend bool operator>=(const tagged_ptr &a, const tagged_ptr &b) noexcept {
    return a.value >= b.value;
  }

  friend void swap(tagged_ptr &a, tagged_ptr &b) noexcept { a.swap(b); }

  template <typename U, unsigned M> friend class tagged_ptr;

  // templated friend function cannot be implemented inline
  template <typename U, unsigned M, class... Args>
  friend tagged_ptr<U, M> make_tagged_ptr(Args &&...);

  // templated friend function cannot be implemented inline
  template <typename U, unsigned M, class... Args>
  friend tagged_ptr<U[], M> make_tagged_array(std::size_t, Args &&...);

  bits_type value;
};

template <typename T, unsigned Nbits, class... Args>
tagged_ptr<T, Nbits> make_tagged_ptr(Args &&... args) {
  tagged_ptr<T, Nbits> p;
  auto address = ::boost::alignment::aligned_alloc(
      detail::max(detail::pow2(Nbits),
                  ::boost::alignment::alignment_of<T>::value),
      sizeof(T));
  p.value = reinterpret_cast<decltype(p.value)>(address);
  new (address) T(std::forward<Args>(args)...);
  return p;
}

template <typename T, unsigned Nbits, class... Args>
tagged_ptr<T[], Nbits> make_tagged_array(std::size_t size, Args &&... args) {
  tagged_ptr<T[], Nbits> p;
  auto address = reinterpret_cast<char *>(::boost::alignment::aligned_alloc(
      detail::max(detail::pow2(Nbits),
                  ::boost::alignment::alignment_of<T>::value),
      sizeof(T *) + size * sizeof(T)));
  auto iter = reinterpret_cast<T *>(address + sizeof(T *));
  const auto end = iter + size;
  *reinterpret_cast<T **>(address) = end;
  p.value = reinterpret_cast<decltype(p.value)>(iter);
  while (iter != end)
    new (iter++) T(std::forward<Args>(args)...);
  return p;
}
}

#endif
