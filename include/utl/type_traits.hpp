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
    using type = typename std::remove_pointer<typename std::remove_cv<typename std::remove_reference<T>::type>::type>::type;
  };

  template <typename T>
  using base_type_t = typename base_type<T>::type;

  template <typename T>
  using is_pointer = std::is_pointer<T>;

  template <typename T>
  constexpr bool is_pointer_v = is_pointer<T>::value;

  template <bool B, typename T = void>
  using enable_if = std::enable_if<B, T>;

  template <bool B, typename T = void>
  using enable_if_t = typename enable_if<B, T>::type;
};
