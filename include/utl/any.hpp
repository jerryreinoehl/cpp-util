#pragma once

#include <cassert>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace utl {

  struct any {
    public:
      template <typename T>
      using is_small_object = std::integral_constant<bool, sizeof(T) <= sizeof(void*) && alignof(T) <= alignof(void*)>;

      template <typename T, typename U>
      using small_object_impl_t = typename std::enable_if<is_small_object<T>::value, U>::type;

      template <typename T, typename U>
      using alloc_impl_t = typename std::enable_if<!is_small_object<T>::value, U>::type;

      template <typename T>
      using decay_t = typename std::decay<T>::type;

      template <typename T, typename... Args>
      typename std::enable_if<std::is_constructible<T, Args...>::value, any>::type
      static make(Args... args);

      // Construct an empty `any` object.
      any() noexcept;

      // Construct an `any` object by copying/moving `t`.
      template <typename T, small_object_impl_t<T, int> = 0>
      any(T&& t);

      template <typename T, alloc_impl_t<T, int> = 0>
      any(T&& t);

      // Copy/move construct another `any`.
      any(const any& other);
      any(any&& other) noexcept;

      // Copy/move assign another `any`.
      any& operator=(const any& other);
      any& operator=(any&& other) noexcept;

      ~any() noexcept;

      // Construct a `T` in place within the `any`.
      template <typename T, typename... Args>
      typename std::enable_if<
        std::is_constructible<T, Args...>::value && is_small_object<T>::value, decay_t<T>&
      >::type
      emplace(Args... args);

      template <typename T, typename... Args>
      typename std::enable_if<
        std::is_constructible<T, Args...>::value && !is_small_object<T>::value, decay_t<T>&
      >::type
      emplace(Args... args);

      // Get a reference/copy of the contained `T`.
      // Asserts that the type of the contained `any` type is `T`.
      template <typename T>
      small_object_impl_t<T, const decay_t<T>&>
      cast() const&;

      template <typename T>
      alloc_impl_t<T, const decay_t<T>&>
      cast() const&;

      template <typename T>
      small_object_impl_t<T, decay_t<T>&>
      cast() &;

      template <typename T>
      alloc_impl_t<T, decay_t<T>&>
      cast() &;

      template <typename T>
      small_object_impl_t<T, decay_t<T>>
      cast() &&;

      template <typename T>
      alloc_impl_t<T, decay_t<T>>
      cast() &&;

      // Destructs the contained type is one exists.
      void reset() noexcept;

      // Returns the type info of the contained type.
      const std::type_info& type() const noexcept;

      // Returns `true` if there is a contained type.
      bool has_value() const noexcept;

      // Returns `true` if there is a contained type and the type is of type `T`.
      template <typename T>
      bool is() const noexcept;

    private:
      enum class manager_op;

      union storage {
        void *external;
        char internal[sizeof(void*)/sizeof(char)];
      } data_{};

      void (*manager_)(manager_op, storage, storage *, const void**){};

      enum class manager_op {
        del,       // delete object
        clone,     // clone object
        type_info, // return type info
      };

      template <typename T, any::alloc_impl_t<T, int> = 0>
      static void any_manager(manager_op op, storage a, storage *b, const void **c);

      template <typename T, any::small_object_impl_t<T, int> = 0>
      static void any_manager(manager_op op, storage a, storage *b, const void **c);
  };


  struct any_view {
    public:
      template <typename T>
      any_view(T&& t);

      template <typename T>
      const T& cast() const;

      // Returns the type info of the contained type.
      const std::type_info& type() const noexcept;

      // Returns `true` if the referenced type is of type `T`.
      template <typename T>
      bool is() const noexcept;

    private:
      const void *data_{};
      const std::type_info *info_{};
  };

}

//*****************************************************************************
// struct utl::any
//*****************************************************************************

template <typename T, typename... Args>
typename std::enable_if<std::is_constructible<T, Args...>::value, utl::any>::type
inline utl::any::make(Args... args) {
  return any{T{std::forward<Args>(args)...}};
}

inline utl::any::any() noexcept : data_{}, manager_{} {}

template <typename T, utl::any::small_object_impl_t<T, int>>
inline utl::any::any(T&& t) {
  using Type = typename std::decay<T>::type;
  new (data_.internal) Type{std::forward<T>(t)};
  manager_ = any_manager<Type>;
}

template <typename T, utl::any::alloc_impl_t<T, int>>
inline utl::any::any(T&& t) {
  using Type = typename std::decay<T>::type;
  data_.external = new Type{std::forward<T>(t)};
  manager_ = any_manager<Type>;
}

inline utl::any::any(const any& other) : manager_{other.manager_} {
  other.manager_(manager_op::clone, other.data_, &data_, nullptr);
}

inline utl::any::any(any&& other) noexcept : data_{other.data_}, manager_{other.manager_} {
  other.data_.external = nullptr;
  other.manager_ = nullptr;
}

inline utl::any& utl::any::operator=(const any& other) {
  if (this == &other) {
    return *this;
  }

  manager_(manager_op::del, data_, nullptr, nullptr);
  other.manager_(manager_op::clone, other.data_, &data_, nullptr);
  manager_ = other.manager_;

  return *this;
}

inline utl::any& utl::any::operator=(any&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  manager_(manager_op::del, data_, nullptr, nullptr);
  data_ = other.data_;
  manager_ = other.manager_;
  other.data_.external = nullptr;
  other.manager_ = nullptr;

  return *this;
}

inline utl::any::~any() noexcept {
  if (manager_ != nullptr) {
    manager_(manager_op::del, data_, nullptr, nullptr);
    data_.external = nullptr;
    manager_ = nullptr;
  }
}

template <typename T, typename... Args>
typename std::enable_if<
  std::is_constructible<T, Args...>::value && utl::any::is_small_object<T>::value, utl::any::decay_t<T>&
>::type
inline utl::any::emplace(Args... args) {
  if (manager_ != nullptr) {
    manager_(manager_op::del, data_, nullptr, nullptr);
  }

  manager_ = any_manager<T>;
  new (data_.internal) T{std::forward<Args>(args)...};

  return *reinterpret_cast<T*>(data_.internal);
}

template <typename T, typename... Args>
typename std::enable_if<
  std::is_constructible<T, Args...>::value && !utl::any::is_small_object<T>::value, utl::any::decay_t<T>&
>::type
inline utl::any::emplace(Args... args) {
  if (manager_ != nullptr) {
    manager_(manager_op::del, data_, nullptr, nullptr);
  }

  manager_ = any_manager<T>;
  data_.external = new T{std::forward<Args>(args)...};

  return *reinterpret_cast<T*>(data_.external);
}

template <typename T>
utl::any::small_object_impl_t<T, const utl::any::decay_t<T>&>
inline utl::any::cast() const& {
  assert(typeid(T) == type());
  return *reinterpret_cast<const T*>(data_.internal);
}

template <typename T>
utl::any::alloc_impl_t<T, const utl::any::decay_t<T>&>
inline utl::any::cast() const& {
  assert(typeid(T) == type());
  return *reinterpret_cast<T*>(data_.external);
}

template <typename T>
utl::any::small_object_impl_t<T, utl::any::decay_t<T>&>
inline utl::any::cast() & {
  assert(typeid(T) == type());
  return *reinterpret_cast<T*>(data_.internal);
}

template <typename T>
utl::any::alloc_impl_t<T, utl::any::decay_t<T>&>
inline utl::any::cast() & {
  assert(typeid(T) == type());
  return *reinterpret_cast<T*>(data_.external);
}

template <typename T>
utl::any::small_object_impl_t<T, utl::any::decay_t<T>>
inline utl::any::cast() && {
  assert(typeid(T) == type());
  return T{std::move(*reinterpret_cast<T*>(data_.internal))};
}

template <typename T>
utl::any::alloc_impl_t<T, utl::any::decay_t<T>>
inline utl::any::cast() && {
  assert(typeid(T) == type());
  return T{std::move(*reinterpret_cast<T*>(data_.external))};
}

inline void utl::any::reset() noexcept {
  this->~any();
}

inline const std::type_info& utl::any::type() const noexcept {
  const void *info;
  manager_(manager_op::type_info, storage{}, nullptr, &info);
  return *reinterpret_cast<const std::type_info*>(info);
}

inline bool utl::any::has_value() const noexcept {
  return manager_ != nullptr;
}

template <typename T>
inline bool utl::any::is() const noexcept {
  return has_value() && (type() == typeid(T));
}

template <typename T, utl::any::alloc_impl_t<T, int>>
void utl::any::any_manager(utl::any::manager_op op, utl::any::storage a, utl::any::storage *b, const void **c) {
  switch (op) {
    case utl::any::manager_op::del:
      delete reinterpret_cast<T*>(a.external);
      break;
    case utl::any::manager_op::clone:
      b->external = new T{*reinterpret_cast<T*>(a.external)};
      break;
    case utl::any::manager_op::type_info:
      *c = &typeid(T);
      break;
  }
}

template <typename T, utl::any::small_object_impl_t<T, int>>
void utl::any::any_manager(utl::any::manager_op op, utl::any::storage a, utl::any::storage *b, const void **c) {
  switch (op) {
    case utl::any::manager_op::del:
      reinterpret_cast<T*>(a.internal)->~T();
      break;
    case utl::any::manager_op::clone:
      new (b->internal) T{*reinterpret_cast<T*>(a.internal)};
      break;
    case utl::any::manager_op::type_info:
      *c = &typeid(T);
      break;
  }
}

//*****************************************************************************
// struct utl::any_view
//*****************************************************************************

template <typename T>
inline utl::any_view::any_view(T&& t) : data_{&t} {
  using Type = typename std::decay<T>::type;
  info_ = &typeid(Type);
}

template <typename T>
inline const T& utl::any_view::cast() const {
  assert(typeid(T) == *info_);
  return *reinterpret_cast<const T*>(data_);
}

inline const std::type_info& utl::any_view::type() const noexcept {
  return *info_;
}

template <typename T>
inline bool utl::any_view::is() const noexcept {
  return type() == *info_;
}
