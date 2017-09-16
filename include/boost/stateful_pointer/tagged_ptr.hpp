#ifndef BOOST_STATEFUL_POINTER_TAGGED_PTR_HPP
#define BOOST_STATEFUL_POINTER_TAGGED_PTR_HPP

#include "boost/align/aligned_alloc.hpp"
#include "boost/assert.hpp"
#include "boost/cstdint.hpp"
#include "boost/type_traits.hpp"

namespace boost {
namespace stateful_pointer {

namespace detail {
constexpr unsigned pow2(unsigned n) noexcept {
  return n > 0 ? 2 * pow2(n - 1) : 1;
}
constexpr uintptr_t make_ptr_mask(unsigned n) noexcept { return ~uintptr_t(0) << n; }
}

template <typename Tpointee, unsigned Nbits> class tagged_ptr {
public:
  using bits_type = uintptr_t;
  using element_type = Tpointee;
  using pointer = Tpointee *;
  using const_pointer = Tpointee const *;
  using reference = Tpointee &;
  using const_reference = Tpointee const &;

  constexpr tagged_ptr() noexcept : ptr(0) {}

  // no copy
  tagged_ptr(const tagged_ptr &) = delete;
  tagged_ptr &operator=(const tagged_ptr &) = delete;

  tagged_ptr(tagged_ptr &&other) noexcept : ptr(other.ptr) { other.ptr = 0; }

  tagged_ptr &operator=(tagged_ptr &&other) noexcept {
    if (this != &other) {
      ptr = other.ptr;
      other.ptr = 0;
    }
    return *this;
  }

  /// move constructor that allows conversion between base and derived
  template <typename Upointee,
            typename = typename enable_if_c<
                !(is_array<Upointee>::value) &&
                is_convertible<Upointee *, Tpointee *>::value>::type>
  tagged_ptr(tagged_ptr<Upointee, Nbits> &&other) noexcept : ptr(other.ptr) {
    other.ptr = 0;
  }

  /// move assignment that allows conversion between base and derived
  template <typename Upointee,
            typename = typename enable_if_c<
                !(is_array<Upointee>::value) &&
                is_convertible<Upointee *, Tpointee *>::value>::type>
  tagged_ptr &operator=(tagged_ptr<Upointee, Nbits> &&other) noexcept {
    if (this != &other) {
      ptr = other.ptr;
      other.ptr = 0;
    }
    return *this;
  }

  ~tagged_ptr() {
    auto p = get();
    if (p) {
      p->~Tpointee();
      alignment::aligned_free(p);
    }
  }

  /// get all bits as integral type
  bits_type bits() const noexcept { return ptr & tag_mask; }

  /// set all bits via integral type
  void bits(bits_type b) noexcept {
    ptr &= ptr_mask;       // clear old bits
    ptr |= (b & tag_mask); // set new bits
  }

  /// get bit at position pos
  bool bit(unsigned pos) const noexcept { return ptr & (1 << pos); }

  /// set bit at position pos to value b
  void bit(unsigned pos, bool b) noexcept {
    BOOST_ASSERT(pos < Nbits);
    if (b)
      ptr |= (1 << pos);
    else
      ptr &= ~(1 << pos);
  }

  /// get raw pointer
  pointer get() noexcept { return extract_ptr(ptr); }

  /// get raw pointer (const version)
  const_pointer get() const noexcept { return extract_ptr(ptr); }

  /// release ownership of raw pointer, bits remain intact
  pointer release() noexcept {
    auto tmp = get();
    ptr &= ~ptr_mask; // nullify pointer bits
    return tmp;
  }

  /// reset ptr and bits to p
  void reset(tagged_ptr p = tagged_ptr()) noexcept { p.swap(*this); }

  void swap(tagged_ptr &other) noexcept {
    auto tmp = ptr;
    ptr = other.ptr;
    other.ptr = tmp;
  }

  /// dereference operator
  reference operator*() noexcept { return *get(); }

  /// dereference operator (const version)
  const_reference operator*() const noexcept { return *get(); }

  /// member access operator
  pointer operator->() noexcept { return get(); }

  /// member access operator (const version)
  const_pointer operator->() const noexcept { return get(); }

  explicit operator bool() const noexcept { return static_cast<bool>(get()); }

  bool operator!() const noexcept { return get() == 0; }

private:
  static constexpr bits_type ptr_mask = detail::make_ptr_mask(Nbits);
  static constexpr bits_type tag_mask = ~ptr_mask;

  static bits_type alloc() {
    return reinterpret_cast<bits_type>(
        alignment::aligned_alloc(detail::pow2(Nbits), sizeof(Tpointee)));
  }

  static pointer extract_ptr(bits_type p) noexcept {
    return reinterpret_cast<Tpointee *>(p & ptr_mask);
  }

  friend bool operator==(const tagged_ptr &a, const tagged_ptr &b) noexcept {
    return a.ptr == b.ptr;
  }

  friend bool operator!=(const tagged_ptr &a, const tagged_ptr &b) noexcept {
    return a.ptr != b.ptr;
  }

  friend bool operator<(const tagged_ptr &a, const tagged_ptr &b) noexcept {
    return a.ptr < b.ptr;
  }

  friend bool operator<=(const tagged_ptr &a, const tagged_ptr &b) noexcept {
    return a.ptr <= b.ptr;
  }

  friend bool operator>(const tagged_ptr &a, const tagged_ptr &b) noexcept {
    return a.ptr > b.ptr;
  }

  friend bool operator>=(const tagged_ptr &a, const tagged_ptr &b) noexcept {
    return a.ptr >= b.ptr;
  }

  friend void swap(tagged_ptr &a, tagged_ptr &b) noexcept { a.swap(b); }

  template <typename Upointee, unsigned Mbits> friend class tagged_ptr;

  // templated friend function cannot be implemented inline
  template <typename Upointee, unsigned Mbits, class... UArgs>
  friend tagged_ptr<Upointee, Mbits> make_tagged_ptr(UArgs &&...);

  bits_type ptr;
};

template <typename Tpointee, unsigned Nbits, class... TArgs>
tagged_ptr<Tpointee, Nbits> make_tagged_ptr(TArgs &&... args) {
  tagged_ptr<Tpointee, Nbits> p;
  p.ptr = p.alloc();
  new (reinterpret_cast<Tpointee *>(p.ptr))
      Tpointee(std::forward<TArgs>(args)...);
  return p;
}
}
}

#endif
