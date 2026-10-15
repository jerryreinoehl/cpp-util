#pragma once

#include <iostream>

namespace utl {

  struct string_view {
    public:
      constexpr string_view() noexcept;
      constexpr string_view(const char *s) noexcept;
      string_view(const std::string& s) noexcept;
      string_view(const string_view& other) noexcept = default;

      string_view& operator=(const string_view& other) = default;

      bool operator==(string_view other) const noexcept;
      bool operator<(string_view other) const noexcept;
      constexpr const char& operator[](size_t idx) const noexcept;

      constexpr const char *data() const noexcept;

      explicit operator std::string() const;

    private:
      const char *data_;
  };

  std::ostream& operator<<(std::ostream& out, utl::string_view s);

}

//*****************************************************************************

inline std::ostream& utl::operator<<(std::ostream& out, utl::string_view s) {
  return out << s.data();
}

//*****************************************************************************
//  struct string_view
//*****************************************************************************

constexpr utl::string_view::string_view() noexcept : data_{nullptr} {}
constexpr utl::string_view::string_view(const char *s) noexcept : data_{s} {}

inline utl::string_view::string_view(const std::string& s) noexcept : data_{s.c_str()} {}

inline bool utl::string_view::operator==(utl::string_view other) const noexcept {
  const char *p{data_}, *q{other.data_};
  for (; *p && *q && *p == *q; ++p, ++q);
  return *p == *q;
}

inline bool utl::string_view::operator<(utl::string_view other) const noexcept {
  const char *p{data_}, *q{other.data_};
  for (; *p && *q && *p == *q; ++p, ++q);
  return *p < *q;
}

constexpr const char& utl::string_view::operator[](size_t idx) const noexcept {
  return data_[idx];
}

constexpr const char* utl::string_view::data() const noexcept {
  return data_;
}

inline utl::string_view::operator std::string() const {
  return std::string{data_};
}
