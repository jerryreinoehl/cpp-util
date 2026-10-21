#pragma once

#include <cassert>

#include <iostream> // REMOVE

namespace utl {

  enum class manager_op {
    Delete,
    Clone,
    TypeInfo,
  };

  struct any {
    public:
      template <typename T>
      using is_small_object = std::integral_constant<bool, sizeof(T) <= sizeof(void*) && alignof(T) <= alignof(void*)>;

      template <typename T, typename U>
      using small_object_impl = std::enable_if<is_small_object<T>::value, U>;

      template <typename T, typename U>
      using small_object_impl_t = typename small_object_impl<T, U>::type;

      template <typename T, typename U>
      using alloc_impl = std::enable_if<!is_small_object<T>::value, U>;

      template <typename T, typename U>
      using alloc_impl_t = typename alloc_impl<T, U>::type;

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

      void reset() noexcept;

      const std::type_info& type() const noexcept;

      bool has_value() const noexcept;

      template <typename T>
      bool is() const noexcept;

    private:
      void *data_{};
      void (*manager_)(manager_op, void *, void **, const void**);

  };

  template <typename T, any::alloc_impl_t<T, int> = 0>
  void any_manager(manager_op op, void *a, void **b, const void **c);

  template <typename T, any::small_object_impl_t<T, int> = 0>
  void any_manager(manager_op op, void *a, void **b, const void **c);

}

template <typename T, utl::any::alloc_impl_t<T, int>>
void utl::any_manager(utl::manager_op op, void *a, void **b, const void **c) {
  switch (op) {
    case utl::manager_op::Delete:
      std::cout << "Deleting any<" << typeid(T).name() << "> @" << a << '\n';
      delete static_cast<T*>(a);
      break;
    case utl::manager_op::Clone:
      *b = new T{*static_cast<T*>(a)};
      std::cout << "Clone any<" << typeid(T).name() << "> @" << a << " -> @" << *b << '\n';
      break;
    case utl::manager_op::TypeInfo:
      *c = &typeid(T);
      break;
  }
}

template <typename T, utl::any::small_object_impl_t<T, int>>
void utl::any_manager(utl::manager_op op, void *a, void **b, const void **c) {
  switch (op) {
    case utl::manager_op::Delete:
      std::cout << "SOO Deleting any<" << typeid(T).name() << "> @" << a << '\n';
      reinterpret_cast<T*>(&a)->~T();
      break;
    case utl::manager_op::Clone:
      std::cout << "SOO Clone any<" << typeid(T).name() << "> @" << a << " -> @" << *b << '\n';
      new (b) T{*reinterpret_cast<T*>(&a)};
      break;
    case utl::manager_op::TypeInfo:
      *c = &typeid(T);
      break;
  }
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
  std::cout << "SOO available\n";

  using Type = typename std::decay<T>::type;
  new (&data_) Type{std::forward<T>(t)};
  manager_ = any_manager<Type>;

  std::cout << "data_ = " << data_ << '\n';
}

template <typename T, utl::any::alloc_impl_t<T, int>>
inline utl::any::any(T&& t) {
  using Type = typename std::decay<T>::type;
  data_ = new Type{std::forward<T>(t)};
  manager_ = any_manager<Type>;

  std::cout << "Allocating new any<" << typeid(T).name() << "> @" << data_ << '\n';
}

inline utl::any::any(const any& other) : manager_{other.manager_} {
  std::cout << "In copy ctor\n";
  other.manager_(manager_op::Clone, other.data_, &data_, nullptr);
}

inline utl::any::any(any&& other) noexcept : data_{other.data_}, manager_{other.manager_} {
  std::cout << "In move ctor\n";
  other.data_ = nullptr;
  other.manager_ = nullptr;
}

inline utl::any& utl::any::operator=(const any& other) {
  std::cout << "In copy assign\n";

  if (this == &other) {
    return *this;
  }

  manager_(manager_op::Delete, data_, nullptr, nullptr);
  other.manager_(manager_op::Clone, other.data_, &data_, nullptr);
  manager_ = other.manager_;

  return *this;
}

inline utl::any& utl::any::operator=(any&& other) noexcept {
  std::cout << "In move assign\n";

  if (this == &other) {
    return *this;
  }

  manager_(manager_op::Delete, data_, nullptr, nullptr);
  data_ = other.data_;
  manager_ = other.manager_;
  other.data_ = nullptr;
  other.manager_ = nullptr;

  return *this;
}

inline utl::any::~any() noexcept {
  std::cout << "In dtor\n";
  if (data_ != nullptr) {
    manager_(manager_op::Delete, data_, nullptr, nullptr);
    data_ = nullptr;
    manager_ = nullptr;
  }
}

template <typename T, typename... Args>
typename std::enable_if<
  std::is_constructible<T, Args...>::value && utl::any::is_small_object<T>::value, utl::any::decay_t<T>&
>::type
inline utl::any::emplace(Args... args) {
  if (manager_ != nullptr) {
    manager_(manager_op::Delete, data_, nullptr, nullptr);
  }

  manager_ = any_manager<T>;
  new (&data_) T{std::forward<Args>(args)...};

  return *reinterpret_cast<T*>(&data_);
}

template <typename T, typename... Args>
typename std::enable_if<
  std::is_constructible<T, Args...>::value && !utl::any::is_small_object<T>::value, utl::any::decay_t<T>&
>::type
inline utl::any::emplace(Args... args) {
  if (data_ != nullptr) {
    manager_(manager_op::Delete, data_, nullptr, nullptr);
  }

  manager_ = any_manager<T>;
  data_ = new T{std::forward<Args>(args)...};

  return *static_cast<T*>(data_);
}

template <typename T>
utl::any::small_object_impl_t<T, const utl::any::decay_t<T>&>
inline utl::any::cast() const& {
  assert(typeid(T) == type());
  return *reinterpret_cast<const T*>(&data_);
}

template <typename T>
utl::any::alloc_impl_t<T, const utl::any::decay_t<T>&>
inline utl::any::cast() const& {
  assert(typeid(T) == type());
  return *static_cast<T*>(data_);
}

template <typename T>
utl::any::small_object_impl_t<T, utl::any::decay_t<T>&>
inline utl::any::cast() & {
  assert(typeid(T) == type());
  return *reinterpret_cast<T*>(&data_);
}

template <typename T>
utl::any::alloc_impl_t<T, utl::any::decay_t<T>&>
inline utl::any::cast() & {
  assert(typeid(T) == type());
  return *static_cast<T*>(data_);
}

template <typename T>
utl::any::small_object_impl_t<T, utl::any::decay_t<T>>
inline utl::any::cast() && {
  assert(typeid(T) == type());
  return T{std::move(*reinterpret_cast<T*>(&data_))};
}

template <typename T>
utl::any::alloc_impl_t<T, utl::any::decay_t<T>>
inline utl::any::cast() && {
  assert(typeid(T) == type());
  return T{std::move(*static_cast<T*>(data_))};
}

inline void utl::any::reset() noexcept {
  this->~any();
}

inline const std::type_info& utl::any::type() const noexcept {
  const void *info;
  manager_(manager_op::TypeInfo, nullptr, nullptr, &info);
  return *static_cast<const std::type_info*>(info);
}

inline bool utl::any::has_value() const noexcept {
  return manager_ != nullptr;
}

template <typename T>
inline bool utl::any::is() const noexcept {
  return has_value() && (type() == typeid(T));
}
