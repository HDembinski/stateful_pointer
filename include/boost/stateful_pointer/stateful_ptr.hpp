#ifndef BOOST_STATEFUL_POINTER_STATEFUL_PTR_HPP
#define BOOST_STATEFUL_POINTER_STATEFUL_PTR_HPP

#include "boost/align/aligned_alloc.hpp"
#include "boost/assert.hpp"
#include "cstdint"

namespace boost {
namespace stateful_pointer {

namespace detail {

constexpr unsigned pow2(unsigned n) noexcept {
  return n > 0 ? 2 * pow2(n - 1) : 1;
}
constexpr std::uintptr_t ptr_mask(unsigned n) noexcept {
  return ~std::uintptr_t(0) << n;
}
}

template <typename Tpointee, unsigned Nbits> class stateful_ptr {
public:
  stateful_ptr() noexcept : ptr(0) {}

  stateful_ptr(const stateful_ptr &other) : ptr(alloc()) {
    new (get()) Tpointee(*other.get()); // copy pointee
    ptr |= other.ptr & ~ptr_mask;       // copy bits
  }

  stateful_ptr &operator=(stateful_ptr &other) {
    if (this != &other) {
      auto *tp = get();
      if (tp) {
        ptr &= ptr_mask; // clear bits
        *tp = *other;    // copy pointee
      } else {
        ptr = alloc();                // bits are also cleared
        new (get()) Tpointee(*other); // copy pointee
      }
      ptr |= other.ptr & ~ptr_mask; // copy bits
    }
    return *this;
  }

  stateful_ptr(stateful_ptr &&other) noexcept : ptr(other.ptr) {
    other.ptr = 0;
  }

  stateful_ptr &operator=(stateful_ptr &&other) noexcept {
    if (this != &other) {
      ptr = other.ptr;
      other.ptr = 0;
    }
    return *this;
  }

  stateful_ptr &operator=(std::nullptr_t) noexcept { reset(); }

  stateful_ptr(const Tpointee &t) : ptr(alloc()) {
    new (get()) Tpointee(t); // copy pointee
  }

  ~stateful_ptr() { alignment::aligned_free(get()); }

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
  Tpointee *get() noexcept { return extract_ptr(ptr); }

  /// get raw pointer (const version)
  Tpointee const *get() const noexcept { return extract_ptr(ptr); }

  /// release ownership of raw pointer, bits remain intact
  Tpointee *release() noexcept {
    auto tmp = get();
    ptr &= ~ptr_mask;
    return tmp;
  }

  /// reset ptr and bits to p
  void reset(stateful_ptr p = stateful_ptr()) noexcept { p.swap(*this); }

  /// reset ptr and bits
  void reset(std::nullptr_t p = nullptr) noexcept { reset(); }

  void swap(stateful_ptr &other) noexcept {
    auto tmp = ptr;
    ptr = other.ptr;
    other.ptr = tmp;
  }

  /// dereference operator
  Tpointee &operator*() noexcept { return *get(); }

  /// dereference operator (const version)
  Tpointee const &operator*() const noexcept { return *get(); }

  /// member access operator
  Tpointee *operator->() noexcept { return get(); }

  /// member access operator (const version)
  Tpointee const *operator->() const noexcept { return get(); }

  operator bool() const noexcept { return static_cast<bool>(get()); }

private:
  static const uintptr_t ptr_mask = detail::ptr_mask(Nbits);

  static uintptr_t alloc() {
    return reinterpret_cast<uintptr_t>(
        alignment::aligned_alloc(detail::pow2(Nbits), sizeof(Tpointee)));
  }

  static Tpointee *extract_ptr(uintptr_t p) noexcept {
    return reinterpret_cast<Tpointee *>(p & ptr_mask);
  }

  friend bool operator==(stateful_ptr a, stateful_ptr b) {
    return a.ptr == b.ptr;
  }

  friend bool operator!=(stateful_ptr a, stateful_ptr b) {
    return a.ptr != b.ptr;
  }

  friend bool operator<(stateful_ptr a, stateful_ptr b) {
    return a.ptr < b.ptr;
  }

  friend bool operator<=(stateful_ptr a, stateful_ptr b) {
    return a.ptr <= b.ptr;
  }

  friend bool operator>(stateful_ptr a, stateful_ptr b) {
    return a.ptr > b.ptr;
  }

  friend bool operator>=(stateful_ptr a, stateful_ptr b) {
    return a.ptr >= b.ptr;
  }

  friend void swap(stateful_ptr a, stateful_ptr b) { a.swap(b); }

  uintptr_t ptr;
};
}
}

#endif
