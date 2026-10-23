#pragma once

#include <type_traits>

namespace utl {

  // Removes const, volatile, and references from type `T`.
  template <typename T>
  struct remove_cvref {
    using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
  };

  template <typename T>
  using remove_cvref_t = typename remove_cvref<T>::type;

  // Obtains the base type of `T`, without const, volatile, references, or pointers.
  template <typename T>
  struct base_type {
    using type = typename std::remove_cv<typename std::remove_reference<typename std::remove_pointer<T>::type>::type>::type;
  };

  template <typename T>
  using base_type_t = typename base_type<T>::type;

  template <typename T>
  using is_pointer = std::is_pointer<T>;

  template <typename T>
  constexpr bool is_pointer_v() { return is_pointer<T>::value; }

  template <typename T>
  using decay = std::decay<T>;

  template <typename T>
  using decay_t = typename std::decay<T>::type;

  template <bool B, typename T = void>
  using enable_if = std::enable_if<B, T>;

  template <bool B, typename T = void>
  using enable_if_t = typename enable_if<B, T>::type;

  template <typename T, typename... Args>
  using is_constructible = std::is_constructible<T, Args...>;

  template <typename T, typename... Args>
  constexpr bool is_constructible_v() { return is_constructible<T, Args...>::value; }

  template <typename T, typename... Args>
  using is_nothrow_constructible = std::is_nothrow_constructible<T, Args...>;

  template <typename T, typename... Args>
  constexpr bool is_nothrow_constructible_v() { return is_nothrow_constructible<T, Args...>::value; }

  template <typename T, typename... Args>
  using is_trivially_constructible = std::is_trivially_constructible<T, Args...>;

  template <typename T, typename... Args>
  constexpr bool is_trivially_constructible_v() { return is_trivially_constructible<T, Args...>::value; }

  // `is_none_of<T, Types...>::value` is `true` if type `T` is not amoung the
  // types in `Types`.
  template <typename T, typename... Types>
  struct is_none_of : std::true_type {};

  template <typename T, typename First, typename... Rest>
  struct is_none_of<T, First, Rest...>
    : std::integral_constant<
      bool,
      !std::is_same<T, First>::value && is_none_of<T, Rest...>::value
    > {};

  // Convenience function for `is_none_of<T, Types...>::value`.
  template <typename T, typename... Types>
  constexpr bool is_none_of_v() {
    return is_none_of<T, Types...>::value;
  }

};
