#pragma once

#include <string>
#include <functional>

namespace utl {

  // An iterator over a sequence of `CharT` characters.
  template <typename CharT>
  struct char_iterator_base {
    public:
      char_iterator_base();
      explicit char_iterator_base(CharT* p);
      char_iterator_base(const char_iterator_base<CharT>& other) = default;

      char_iterator_base<CharT>& operator=(const char_iterator_base<CharT>& other) = default;

      // Returns a reference ot the `CharT` the iterator currently points to.
      CharT& operator*() noexcept;

      // Advances the iterator only if it does not point to a null char.
      char_iterator_base<CharT>& operator++() noexcept;
      char_iterator_base<CharT> operator++(int) noexcept;

      // Two `char_iterator_base`'s are equivalent if they point to the same
      // memory location.
      bool operator==(char_iterator_base<CharT> other) const noexcept;
      bool operator!=(char_iterator_base<CharT> other) const noexcept;

      // Converts to `false` if the iterator points to a null char '\0', and
      // `true` otherwise.
      operator bool() const noexcept;

      // Convert the iterator to a string starting from its current position
      // and ending at the next null char, or ending at another iterator,
      // `end`.
      std::string to_string() const;
      std::string to_string(char_iterator_base<CharT> end) const;

      // Advance the iterator until it matches a `CharT` `c`, the `CharT`
      // sequence `cp`, or while the function `func` returns `true`.
      char_iterator_base<CharT> seek(CharT c) noexcept;
      char_iterator_base<CharT> seek(CharT *cp) noexcept;
      char_iterator_base<CharT> seek(std::function<bool(CharT)> func);

      // Return `true` if the next sequence of `CharT`'s the iterator points to
      // is equivalent to `cp`.
      bool starts_with(CharT *cp) const noexcept;

    private:
      CharT* p_{};
  };

  using char_iterator = char_iterator_base<char>;
  using const_char_iterator = char_iterator_base<const char>;

}

//*****************************************************************************
// template <typename CharT>
// struct utl::char_iterator_base<CharT>
//*****************************************************************************

template <typename CharT>
inline utl::char_iterator_base<CharT>::char_iterator_base() : p_{nullptr} {}

template <typename CharT>
inline utl::char_iterator_base<CharT>::char_iterator_base(CharT* p) : p_{p} {}

template <typename CharT>
inline CharT& utl::char_iterator_base<CharT>::operator*() noexcept {
  return *p_;
}

template <typename CharT>
inline utl::char_iterator_base<CharT>& utl::char_iterator_base<CharT>::operator++() noexcept {
  if (*p_) ++p_;
  return *this;
}

template <typename CharT>
inline utl::char_iterator_base<CharT> utl::char_iterator_base<CharT>::operator++(int) noexcept {
  auto tmp{*this};
  if (*p_) ++p_;
  return tmp;
}

template <typename CharT>
inline bool utl::char_iterator_base<CharT>::operator==(char_iterator_base<CharT> other) const noexcept {
  return p_ == other.p_;
}

template <typename CharT>
inline bool utl::char_iterator_base<CharT>::operator!=(char_iterator_base<CharT> other) const noexcept {
  return p_ != other.p_;
}

template <typename CharT>
inline utl::char_iterator_base<CharT>::operator bool() const noexcept {
  return *p_;
}

template <typename CharT>
inline std::string utl::char_iterator_base<CharT>::to_string() const {
  CharT *q{p_};
  for (; *q; ++q);
  return std::string{p_, q};
}

template <typename CharT>
inline std::string utl::char_iterator_base<CharT>::to_string(char_iterator_base<CharT> end) const {
  return std::string{p_, end.p_};
}

template <typename CharT>
inline utl::char_iterator_base<CharT> utl::char_iterator_base<CharT>::seek(CharT c) noexcept {
  for (; *p_ && *p_ != c; ++p_);
  return *this;
}

template <typename CharT>
inline utl::char_iterator_base<CharT> utl::char_iterator_base<CharT>::seek(CharT *cp) noexcept {
  for (CharT *q{cp}; *p_ && *q; ++p_) {
    if (*p_ == *q) {
      for (; *p_ && *q && *p_ == *q; ++p_, ++q);
      if (!*q) break;
      q = cp;
    }
  }

  return *this;
}

template <typename CharT>
inline utl::char_iterator_base<CharT> utl::char_iterator_base<CharT>::seek(std::function<bool(CharT)> func) {
  for (; *p_ && func(*p_); ++p_);
  return *this;
}

template <typename CharT>
bool utl::char_iterator_base<CharT>::starts_with(CharT *cp) const noexcept {
  CharT *p{p_}, *q{cp};

  for (; *p && *q; ++p, ++q) {
    if (*p != *q) {
      return false;
    }
  }

  return !*q;
}
