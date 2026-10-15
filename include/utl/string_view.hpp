#pragma once

#include <iostream>

namespace utl {

  struct string_view {
    public:
      constexpr string_view(const char *s) noexcept;
      string_view(const std::string& s) noexcept;

      constexpr const char *data() const noexcept;

      constexpr bool operator<(string_view other) const noexcept;

    private:
      const char *data_;
  };

}

std::ostream& operator<<(std::ostream& out, utl::string_view s);

//*****************************************************************************

inline std::ostream& operator<<(std::ostream& out, utl::string_view s) {
  return out << s.data();
}

//*****************************************************************************
//  struct string_view
//*****************************************************************************

constexpr utl::string_view::string_view(const char *s) noexcept : data_{s} {}

inline utl::string_view::string_view(const std::string& s) noexcept : data_{s.c_str()} {}

constexpr const char* utl::string_view::data() const noexcept {
  return data_;
}

constexpr bool utl::string_view::operator<(utl::string_view other) const noexcept {
  const char *p{data_}, *q{other.data_};
  for(; *p && *q && *p == *q; ++p, ++q);
  return *p < *q;
}
