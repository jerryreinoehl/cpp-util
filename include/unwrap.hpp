#pragma once

#include <type_traits>
#include <utility>

template <typename T, typename = void>
struct error_propagation_traits {};

#define UNWRAP_PROPAGATE(EXPR)                                                             \
  ({                                                                                       \
    auto __res = (EXPR);                                                                   \
    using __restype = std::remove_reference<std::remove_cv<decltype(__res)>::type>::type;  \
    using __traits = error_propagation_traits<__restype>;                                  \
                                                                                           \
    if (!__traits::has_value(__res)) {                                                     \
      return __traits::from_error(__traits::extract_error(std::move(__res)));              \
    }                                                                                      \
    __traits::extract_value(std::move(__res));                                             \
  })

#define UNWRAP_FALLBACK(EXPR, FALLBACK)                                                    \
  ({                                                                                       \
    auto __res = (EXPR);                                                                   \
    using __restype = std::remove_reference<std::remove_cv<decltype(__res)>::type>::type;  \
    using __traits = error_propagation_traits<__restype>;                                  \
                                                                                           \
    if (!__traits::has_value(__res)) {                                                     \
      return (FALLBACK);                                                                   \
    }                                                                                      \
    __traits::extract_value(std::move(__res));                                             \
  })

#define UNWRAP_GET_MACRO(_1, _2, NAME, ...) NAME

#define unwrap(...) UNWRAP_GET_MACRO(__VA_ARGS__, UNWRAP_FALLBACK, UNWRAP_PROPAGATE)(__VA_ARGS__)

template <typename T>
struct error_propagation_traits<T, typename std::enable_if<std::is_convertible<T, bool>::value>::type> {
  static constexpr bool has_value(const T& t) noexcept {
    return static_cast<bool>(t);
  }

  static constexpr T extract_value(const T& t) noexcept {
    return t;
  }

  static constexpr T extract_error(const T& t) noexcept {
    return t;
  }

  static constexpr T from_value(T&& t) noexcept {
    return std::forward<T>(t);
  }

  static constexpr T from_error(T&& t) noexcept {
    return std::forward<T>(t);
  }
};
