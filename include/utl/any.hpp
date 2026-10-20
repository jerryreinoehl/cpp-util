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

      template <typename T, bool B, typename U>
      using small_object_optimization = std::enable_if<is_small_object<T>::value == B, U>;

      template <typename T, bool B, typename U>
      using small_object_optimization_t = typename small_object_optimization<T, B, U>::type;

      template <typename T, typename... Args>
      typename std::enable_if<std::is_constructible<T, Args...>::value, any>::type
      static make(Args... args);

      template <typename T>
      small_object_optimization_t<T, true, const typename std::decay<T>::type&>
      static cast(const any& any);

      template <typename T>
      small_object_optimization_t<T, false, const typename std::decay<T>::type&>
      static cast(const any& any);

      template <typename T>
      small_object_optimization_t<T, true, typename std::decay<T>::type&>
      static cast(any& any);

      template <typename T>
      small_object_optimization_t<T, false, typename std::decay<T>::type&>
      static cast(any& any);

      template <typename T>
      small_object_optimization_t<T, true, typename std::decay<T>::type>
      static cast(any&& any);

      template <typename T>
      small_object_optimization_t<T, false, typename std::decay<T>::type>
      static cast(any&& any);

      any() noexcept;

      template <typename T, small_object_optimization_t<T, true, int> = 0>
      any(const T& t);

      template <typename T, small_object_optimization_t<T, false, int> = 0>
      any(const T& t);

      any(const any& other);

      any(any&& other) noexcept ;

      any& operator=(const any& other);

      any& operator=(any&& other) noexcept;

      ~any() noexcept;

      template <typename T, typename... Args>
      typename std::enable_if<std::is_constructible<T, Args...>::value, typename std::decay<T>::type&>::type
      emplace(Args... args);

      void reset() noexcept;

      const std::type_info& type() const noexcept;

      bool has_value() const noexcept;

    private:
      void *data_{};
      void (*manager_)(manager_op, void *, void **, const void**);

  };

  template <typename T, any::small_object_optimization_t<T, false, int> = 0>
  void any_manager(manager_op op, void *a, void **b, const void **c);

  template <typename T, any::small_object_optimization_t<T, true, int> = 0>
  void any_manager(manager_op op, void *a, void **b, const void **c);

}

template <typename T, utl::any::small_object_optimization_t<T, false, int>>
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

template <typename T, utl::any::small_object_optimization_t<T, true, int>>
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

template <typename T>
utl::any::small_object_optimization_t<T, true, const typename std::decay<T>::type&>
inline utl::any::cast(const any& any) {
  assert(typeid(T) == any.type());
  return *reinterpret_cast<const T*>(&any.data_);
}

template <typename T>
utl::any::small_object_optimization_t<T, false, const typename std::decay<T>::type&>
inline utl::any::cast(const any& any) {
  assert(typeid(T) == any.type());
  return *static_cast<T*>(any.data_);
}

template <typename T>
utl::any::small_object_optimization_t<T, true, typename std::decay<T>::type&>
inline utl::any::cast(any& any) {
  assert(typeid(T) == any.type());
  return *reinterpret_cast<T*>(&any.data_);
}

template <typename T>
utl::any::small_object_optimization_t<T, false, typename std::decay<T>::type&>
inline utl::any::cast(any& any) {
  assert(typeid(T) == any.type());
  return *static_cast<T*>(any.data_);
}

template <typename T>
utl::any::small_object_optimization_t<T, true, typename std::decay<T>::type>
inline utl::any::cast(any&& any) {
  assert(typeid(T) == any.type());
  return T{std::move(*reinterpret_cast<T*>(&any.data_))};
}

template <typename T>
utl::any::small_object_optimization_t<T, false, typename std::decay<T>::type>
inline utl::any::cast(any&& any) {
  assert(typeid(T) == any.type());
  return T{std::move(*static_cast<T*>(any.data_))};
}

inline utl::any::any() noexcept : data_{}, manager_{} {}

template <typename T, utl::any::small_object_optimization_t<T, true, int>>
inline utl::any::any(const T& t) : manager_{any_manager<T>} {
  std::cout << "SOO available\n";
  new (&data_) T{t};
  std::cout << "data_ = " << data_ << '\n';
}

template <typename T, utl::any::small_object_optimization_t<T, false, int>>
inline utl::any::any(const T& t) : manager_{any_manager<T>} {
  data_ = new T{t};
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
typename std::enable_if<std::is_constructible<T, Args...>::value, typename std::decay<T>::type&>::type
inline utl::any::emplace(Args... args) {
  if (data_ != nullptr) {
    manager_(manager_op::Delete, data_, nullptr, nullptr);
  }

  manager_ = any_manager<T>;
  data_ = new T{std::forward<Args>(args)...};

  return *static_cast<T*>(data_);
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
