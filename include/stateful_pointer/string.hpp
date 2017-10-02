#ifndef STATEFUL_POINTER_STRING_HPP
#define STATEFUL_POINTER_STRING_HPP

#include "boost/cstdint.hpp"
#include "stateful_pointer/tagged_ptr.hpp"
// #include "boost/assert.hpp"
// #include "boost/type_traits.hpp"
#include "boost/utility/binary.hpp"
#include <algorithm>
#include <cstddef>
#include <ostream>
#include <stdexcept>

namespace stateful_pointer {

namespace detail {
template <typename T, typename = decltype(std::begin(std::declval<T &>()),
                                          std::end(std::declval<T &>()))>
struct is_sequence {};
} // namespace detail

template <typename TChar> class basic_string {
  using tagged_ptr_t = tagged_ptr<TChar[], 1>;
  using bits_type = typename tagged_ptr_t::bits_type;
  static constexpr unsigned N = sizeof(void *) / sizeof(TChar);

public:
  using pos_type = std::size_t;
  using value_type = typename tagged_ptr_t::element_type;
  using pointer = value_type *;
  using const_pointer = value_type const *;
  using reference = value_type &;
  using const_reference = value_type const &;
  using iterator = pointer;
  using const_iterator = const_pointer;

  constexpr basic_string() noexcept {}

  basic_string(pos_type count, value_type ch) {
    if (N > 0 && count < N) { // small string optimisation
      auto cp = reinterpret_cast<pointer>(&value) + 1;
      std::fill_n(cp, count, ch);
      reinterpret_cast<bits_type &>(value) |= count << 1;
      // value.bit(0) remains false
    } else { // normal use
      value = make_tagged<value_type[], 1>(count + 1);
      value.bit(0, true);
      auto cp = value.get();
      std::fill_n(cp, count, ch);
      *(cp + count) = 0;
    }
  }

  basic_string(const basic_string &other, pos_type pos, pos_type count) {
    auto first = other.begin() + pos;
    assign_impl(first, std::min(other.end(), first + count));
  }

  basic_string(const basic_string &other, pos_type pos) {
    assign_impl(other.begin() + pos, other.end());
  }

  basic_string(const value_type *s, pos_type count) {
    if (count > 0 && !s)
      throw std::logic_error("null in constructor not valid");
    assign_impl(s, s + count);
  }

  basic_string(const value_type *s) {
    if (!s)
      throw std::logic_error("null in constructor not valid");
    auto end = s;
    while (*end++)
      ;
    assign_impl(s, --end);
  }

  template <typename InputIt> basic_string(InputIt first, InputIt last) {
    assign_impl(first, last);
  }

  ~basic_string() {
    if (!value.bit(0)) { // we are in small string optimisation mode
      // prevent tagged_ptr destructor from running
      reinterpret_cast<bits_type &>(value) = 0;
    }
  }

  const_iterator begin() const noexcept {
    return begin_impl<const_iterator>(value);
  }

  const_iterator end() const noexcept {
    return end_impl<const_iterator>(value);
  }

  iterator begin() noexcept { return begin_impl<iterator>(value); }

  iterator end() noexcept { return end_impl<iterator>(value); }

  bool empty() const noexcept {
    return reinterpret_cast<const bits_type &>(value) == 0 || size() == 0;
  }

  pos_type size() const noexcept {
    if (value.bit(0)) {
      auto size = value.size();
      return size ? size - 1 : 0;
    }
    return (reinterpret_cast<const bits_type &>(value) & size_mask) >> 1;
  }

  pos_type length() const noexcept { return size(); }

  bool operator==(const value_type *s) const {
    auto send = s;
    while (*send++)
      ;
    --send;
    auto first = begin();
    auto last = end();
    return (std::distance(first, last) == std::distance(s, send)) &&
           std::equal(first, last, s);
  }

  template <typename Container, typename = detail::is_sequence<Container>>
  bool operator==(const Container &c) const {
    auto cfirst = std::begin(c);
    auto cend = std::end(c);
    auto first = begin();
    auto last = end();
    return (std::distance(first, last) == std::distance(cfirst, cend)) &&
           std::equal(first, last, cfirst);
  }

  const_reference operator[](pos_type i) const { return *(begin() + i); }

  reference operator[](pos_type i) { return *(begin() + i); }

private:
  static constexpr bits_type size_mask = BOOST_BINARY(11111110);

  template <typename InputIt> void assign_impl(InputIt first, InputIt last) {
    const auto n = std::distance(first, last);

    if (value.bit(0) && n <= size()) {
      // normal pointer, reuse allocated memory
      auto cp = value.get();
      std::copy(first, last, cp);
      *(cp + n) = 0;
      return;
    }

    if (!value.bit(0)) {
      // small string optimisation: characters stored inside pointer memory
      reinterpret_cast<bits_type &>(value) = 0; // wipe memory
      if (N > 0 && n < N) {
        // small string optimisation remains possible
        auto cp = reinterpret_cast<pointer>(&value) + 1;
        std::copy(first, last, cp);
        reinterpret_cast<bits_type &>(value) |= n << 1;
        // value.bit(0) remains false
        return;
      }
    }

    // allocate new memory
    // value.bit(0) == true marks normal pointer use
    value = make_tagged<value_type[], 1>(n + 1);
    value.bit(0, true);
    auto cp = value.get();
    std::copy(first, last, cp);
    *(cp + n) = 0;
  }

  template <typename It, typename T> static It begin_impl(T &t) noexcept {
    return t.bit(0) ? t.get() : reinterpret_cast<It>(&t) + 1;
  }

  template <typename It, typename T> static It end_impl(T &t) noexcept {
    if (t.bit(0)) {
      const auto n = t.size();
      return t.get() + (n ? n - 1 : 0);
    } else {
      const auto n = (reinterpret_cast<const bits_type &>(t) & size_mask) >> 1;
      return reinterpret_cast<It>(&t) + 1 + n;
    }
  }

  friend std::ostream &operator<<(std::ostream &os, const basic_string &s) {
    for (const auto &ch : s)
      os << ch;
    return os;
  }

  tagged_ptr_t value;
};

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
} // namespace stateful_pointer

#endif
