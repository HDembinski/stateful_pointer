#ifndef STATEFUL_POINTER_STRING_HPP
#define STATEFUL_POINTER_STRING_HPP

#include "boost/cstdint.hpp"
#include "stateful_pointer/tagged_ptr.hpp"
// #include "boost/assert.hpp"
// #include "boost/type_traits.hpp"
#include "algorithm"
#include "boost/utility/binary.hpp"
#include "cstddef"
#include "stdexcept"

namespace stateful_pointer {

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
      value = make_tagged_array<value_type, 1>(count + 1);
      value.bit(0, true);
      auto cp = value.get();
      std::fill_n(cp, count, ch);
      *(cp + count) = 0;
    }
  }

  basic_string(const basic_string& other, pos_type pos, pos_type count) {
    auto first = other.begin() + pos;
    priv_assign(first, std::min(other.end(), first + count));
  }

  basic_string(const basic_string& other, pos_type pos) {
    priv_assign(other.begin() + pos, other.end());
  }

  basic_string(const value_type *s, pos_type count) {
    if (count > 0 && !s)
      throw std::logic_error("null in constructor not valid");
    priv_assign(s, s + count);
   }

  basic_string(const value_type *s) {
    if (!s)
      throw std::logic_error("null in constructor not valid");
    auto end = s;
    while (*end++);
    priv_assign(s, --end);
  }

  template <typename InputIt>
  basic_string(InputIt first, InputIt last) {
    priv_assign(first, last);
  }

  ~basic_string() {
    if (!value.bit(0)) { // we are in small string optimisation mode
      // prevent tagged_ptr destructor from running
      reinterpret_cast<bits_type &>(value) = 0; 
    }
  }

  const_iterator begin() const noexcept {
    return priv_begin<const_iterator>(value);
  }

  const_iterator end() const noexcept {
    return priv_end<const_iterator>(value);
  }

  iterator begin() noexcept {
    return priv_begin<iterator>(value);
  }

  iterator end() noexcept {
    return priv_end<iterator>(value);
  }

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

  bool operator==(const_pointer s) const {
    auto send = s;
    while (*send++); --send;
    auto first = begin();
    auto last = end();
    return (std::distance(first, last) == std::distance(s, send)) && std::equal(first, last, s);
  }

  const_reference operator[](pos_type i) const {
    return *(begin() + i); 
  }

  reference operator[](pos_type i) {
    return *(begin() + i); 
  }

private:
  static constexpr bits_type size_mask = BOOST_BINARY(11111110);

  template <typename InputIt>
  void priv_assign(InputIt first, InputIt last) {
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
    value = make_tagged_array<value_type, 1>(n + 1);
    value.bit(0, true);
    auto cp = value.get();
    std::copy(first, last, cp);
    *(cp + n) = 0;
  }

  template <typename It, typename T>
  static It priv_begin(T& t) noexcept {
    return t.bit(0) ? t.get() : reinterpret_cast<It>(&t) + 1;
  }

  template <typename It, typename T>
  static It priv_end(T& t) noexcept {
    if (t.bit(0)) {
      const auto n = t.size();
      return t.get() + (n ? n - 1 : 0);
    } else {
      const auto n = (reinterpret_cast<const bits_type &>(t) & size_mask) >> 1;
      return reinterpret_cast<It>(&t) + 1 + n;
    }
  }

  tagged_ptr_t value;
};

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;


}

#endif
