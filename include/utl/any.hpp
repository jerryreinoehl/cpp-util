#pragma once

#include <cassert>

#include <iostream> // REMOVE

namespace utl {

  enum class manager_op {
    Delete,
    Clone,
    TypeInfo,
  };

  template <typename T>
  void any_manager(manager_op op, void *a, void **b, const void **c) {
    switch (op) {
      case manager_op::Delete:
        if (a != nullptr) {
          std::cout << "Deleting any<" << typeid(T).name() << "> @" << a << '\n';
          delete static_cast<T*>(a);
        }
        break;
      case manager_op::Clone:
        *b = new T{*static_cast<T*>(a)};
        std::cout << "Clone any<" << typeid(T).name() << "> @" << a << " -> @" << *b << '\n';
        break;
      case manager_op::TypeInfo:
        *c = &typeid(T);
        break;
    }
  }

  struct any {
    public:
      template <typename T, typename... Args>
      typename std::enable_if<std::is_constructible<T, Args...>::value, any>::type
      static make(Args... args) {
        return any{T{std::forward<Args>(args)...}};
      }

      template <typename T>
      static T& cast(const any& any) {
        assert(typeid(T) == any.type());
        return *static_cast<T*>(any.data_);
      }

      template <typename T>
      static T cast(any&& any) {
        assert(typeid(T) == any.type());
        return T{std::move(*static_cast<T*>(any.data_))};
      }

      template <typename T>
      any(const T& t) : manager_{any_manager<T>} {
        data_ = new T{t};
        std::cout << "Allocating new any<" << typeid(T).name() << "> @" << data_ << '\n';
      }

      any(const any& other) : manager_{other.manager_} {
        std::cout << "In copy ctor\n";
        other.manager_(manager_op::Clone, other.data_, &data_, nullptr);
      }

      any(any&& other) : data_{other.data_}, manager_{other.manager_} {
        std::cout << "In move ctor\n";
        other.data_ = nullptr;
      }

      any& operator=(const any& other) {
        if (this == &other) {
          return *this;
        }

        manager_(manager_op::Delete, data_, nullptr, nullptr);
        other.manager_(manager_op::Clone, other.data_, &data_, nullptr);
        manager_ = other.manager_;

        return *this;
      }

      any& operator=(any&& other) {
        if (this == &other) {
          return *this;
        }

        manager_(manager_op::Delete, data_, nullptr, nullptr);
        data_ = other.data_;
        manager_ = other.manager_;
        other.data_ = nullptr;

        return *this;
      }

      ~any() {
        std::cout << "In dtor\n";
        manager_(manager_op::Delete, data_, nullptr, nullptr);
      }

      const std::type_info& type() const noexcept {
        const void *info;
        manager_(manager_op::TypeInfo, nullptr, nullptr, &info);
        return *static_cast<const std::type_info*>(info);
      }

    private:
      void *data_{};
      void (*manager_)(manager_op, void *, void **, const void**);

  };

}
