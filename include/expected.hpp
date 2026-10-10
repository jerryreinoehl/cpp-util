#pragma once

#include <utility>

template <typename E>
struct unexpected {
  public:
    explicit unexpected(const E& error);
    explicit unexpected(E&& error);
    unexpected(const unexpected<E>& error);
    unexpected(unexpected<E>&& error);

    template <
      typename... Args,
      typename std::enable_if<std::is_constructible<E, Args...>::value, int>::type = 0
    >
    explicit unexpected(Args&&... args);

    unexpected& operator=(const unexpected<E>& rhs);
    unexpected& operator=(unexpected<E>&& rhs);

    const E& error() const& noexcept;
    E&& error() && noexcept;

  private:
    E error_;
};

template <typename V, typename E>
class expected;

template <typename T>
struct expected_traits;

template <typename V, typename E>
struct expected_traits<expected<V, E>> {
  typedef V value_type;
  typedef E error_type;
};

template <typename E>
struct expected_traits<unexpected<E>> {
  typedef E error_type;
};

template <typename T, typename V, typename E>
struct normalize_expected {
  typedef expected<typename std::decay<T>::type, E> type;
};

template <typename U, typename F, typename V, typename E>
struct normalize_expected<expected<U, F>, V, E> {
  typedef expected<U, F> type;
};

template <typename F, typename V, typename E>
struct normalize_expected<unexpected<F>, V, E> {
  typedef expected<V, F> type;
};

template <typename V, typename E>
class expected {
  public:
    expected(const V& value);
    expected(V&& value);

    template <typename... Args>
    expected(Args&&... args);

    expected(const expected<V, E>& expected);
    expected(expected<V, E>&& expected);

    expected(const unexpected<E>& error);
    expected(unexpected<E>&& error);

    ~expected();

    bool has_value() const noexcept;

    V& value() &;
    const V& value() const&;
    V&& value() &&;

    E& error() &;
    const E& error() const&;
    E&& error() &&;

    template <typename F>
    typename normalize_expected<
      typename std::result_of<F(V&)>::type, V, E
    >::type
    and_then(F&& func) &;

    template <typename F>
    typename normalize_expected<
      typename std::result_of<F(V&&)>::type, V, E
    >::type
    and_then(F&& func) &&;

    template <typename F>
    expected<V, E>
    or_else(F&& func) &;

    template <typename F>
    expected<V, E>
    or_else(F&& func) &&;

    template <typename U, typename std::enable_if<std::is_constructible<V, U>::value, int>::type = 0>
    V value_or(U&& fallback) &;

    template <typename U, typename std::enable_if<std::is_constructible<V, U>::value, int>::type = 0>
    V value_or(U&& fallback) &&;

    expected<V, E>& operator=(const expected<V, E>& rhs) noexcept;
    expected<V, E>& operator=(expected<V, E>&& rhs) noexcept;

    explicit operator bool() const;

    const V& operator*() const&;
    V& operator*() &;

    const V* operator->() const&;
    V* operator->() &;

  private:
    union Storage {
      V value;
      E error;

      Storage() {}
      ~Storage() {}
    } storage_;

    bool has_value_{false};

    void destroy() noexcept {
      if (has_value_) {
        storage_.value.~V();
      } else {
        storage_.error.~E();
      }
    }
};

template <typename T, typename>
struct error_propagation_traits;

template <typename V, typename E>
struct error_propagation_traits<expected<V, E>, void> {
  static bool has_value(const expected<V, E>& t) {
    return t.has_value();
  }

  static V extract_value(expected<V, E>&& t) {
    return std::forward<expected<V, E>>(t).value();
  }

  static E extract_error(expected<V, E>&& t) {
    return std::forward<expected<V, E>>(t).error();
  }

  static expected<V, E> from_value(V&& v) {
    return std::forward<V>(v);
  }

  static unexpected<E> from_error(E&& e) {
    return unexpected<E>{std::forward<E>(e)};
  }
};

//*****************************************************************************
// template <typename E>
// struct unexpected<E>
//*****************************************************************************

template <typename E>
inline unexpected<E>::unexpected(const E& error) : error_{error} {}

template <typename E>
inline unexpected<E>::unexpected(E&& error) : error_{std::move(error)} {}

template <typename E>
inline unexpected<E>::unexpected(const unexpected<E>& error) : error_{error.error_} {}

template <typename E>
inline unexpected<E>::unexpected(unexpected<E>&& error) : error_{std::move(error.error_)} {}

template <typename E>
template <
  typename... Args,
  typename std::enable_if<std::is_constructible<E, Args...>::value, int>::type
>
inline unexpected<E>::unexpected(Args&&... args) : unexpected{E{std::forward<Args>(args)...}} {}

template <typename E>
inline unexpected<E>& unexpected<E>::operator=(const unexpected<E>& rhs) {
  if (*this == &rhs) {
    return *this;
  }

  error_ = rhs.error_;

  return *this;
}

template <typename E>
inline unexpected<E>& unexpected<E>::operator=(unexpected<E>&& rhs) {
  if (*this == &rhs) {
    return *this;
  }

  error_ = std::move(rhs.error_);

  return *this;
}

template <typename E>
inline const E& unexpected<E>::error() const& noexcept {
  return error_;
}

template <typename E>
inline E&& unexpected<E>::error() && noexcept {
  return std::move(error_);
}

//*****************************************************************************
// template <typename V, typename E>
// struct expected<T, E>
//*****************************************************************************

template <typename V, typename E>
inline expected<V, E>::expected(const V& value) : has_value_{true} {
  new (&storage_.value) V{value};
}

template <typename V, typename E>
inline expected<V, E>::expected(V&& value) : has_value_{true} {
  new (&storage_.value) V{std::move(value)};
}

template <typename V, typename E>
template <typename... Args>
inline expected<V, E>::expected(Args&&... args) : expected{V{std::forward<Args>(args)...}} {}

template <typename V, typename E>
inline expected<V, E>::expected(const expected<V, E>& expected) : has_value_{expected.has_value_} {
  if (has_value_) {
    new (&storage_.value) V{expected.storage_.value};
  } else {
    new (&storage_.error) E{expected.storage_.error};
  }
}

template <typename V, typename E>
inline expected<V, E>::expected(expected<V, E>&& expected) : has_value_{expected.has_value_} {
  if (has_value_) {
    new (&storage_.value) V{std::move(expected.storage_.value)};
  } else {
    new (&storage_.error) E{std::move(expected.storage_.error)};
  }
}

template <typename V, typename E>
inline expected<V, E>::expected(const unexpected<E>& error) : has_value_{false} {
  new (&storage_.error) E{error.error()};
}

template <typename V, typename E>
inline expected<V, E>::expected(unexpected<E>&& error) : has_value_{false} {
  new (&storage_.error) E{std::move(error.error())};
}

template <typename V, typename E>
inline expected<V, E>::~expected() {
  destroy();
}

template <typename V, typename E>
inline bool expected<V, E>::has_value() const noexcept {
  return has_value_;
}

template <typename V, typename E>
inline V& expected<V, E>::value() & {
  return storage_.value;
}

template <typename V, typename E>
inline const V& expected<V, E>::value() const& {
  return storage_.value;
}

template <typename V, typename E>
inline V&& expected<V, E>::value() && {
  return std::move(storage_.value);
}

template <typename V, typename E>
inline E& expected<V, E>::error() & {
  return storage_.error;
}

template <typename V, typename E>
inline const E& expected<V, E>::error() const& {
  return storage_.error;
}

template <typename V, typename E>
inline E&& expected<V, E>::error() && {
  return std::move(storage_.error);
}

template <typename V, typename E>
template <typename F>
typename normalize_expected<
  typename std::result_of<F(V&)>::type, V, E
>::type
expected<V, E>::and_then(F&& func) & {
  typedef typename std::result_of<F(V&)>::type CallbackResult;
  typedef typename normalize_expected<CallbackResult, V, E>::type Result;

  static_assert(
    std::is_same<typename expected_traits<Result>::error_type, E>::value,
    "and_then callback must return either expected<V, E> or expected<E>"
  );

  if (has_value_) {
    return Result{func(storage_.value)};
  }

  return Result{unexpected<E>(storage_.error)};
}

template <typename V, typename E>
template <typename F>
typename normalize_expected<
  typename std::result_of<F(V&&)>::type, V, E
>::type
expected<V, E>::and_then(F&& func) && {
  typedef typename std::result_of<F(V&&)>::type CallbackResult;
  typedef typename normalize_expected<CallbackResult, V, E>::type Result;

  static_assert(
    std::is_same<typename expected_traits<Result>::error_type, E>::value,
    "and_then callback must return either expected<V, E> or expected<E>"
  );

  if (has_value_) {
    return Result{func(std::move(storage_.value))};
  }

  return Result{unexpected<E>(std::move(storage_.error))};
}

template <typename V, typename E>
template <typename F>
expected<V, E>
expected<V, E>::or_else(F&& func) & {
  typedef typename std::result_of<F(E&)>::type CallbackResult;
  typedef typename normalize_expected<CallbackResult, V, E>::type Result;

  static_assert(
    std::is_same<typename expected_traits<Result>::value_type, V>::value &&
    std::is_same<typename expected_traits<Result>::error_type, E>::value,
    "and_then callback must return either expected<V, E> or expected<E>"
  );

  if (has_value_) {
    return *this;
  }

  return Result{func(storage_.error)};
}

template <typename V, typename E>
template <typename F>
expected<V, E>
expected<V, E>::or_else(F&& func) && {
  typedef typename std::result_of<F(E&&)>::type CallbackResult;
  typedef typename normalize_expected<CallbackResult, V, E>::type Result;

  static_assert(
    std::is_same<typename expected_traits<Result>::value_type, V>::value &&
    std::is_same<typename expected_traits<Result>::error_type, E>::value,
    "and_then callback must return either expected<V, E> or expected<E>"
  );

  if (has_value_) {
    return std::move(*this);
  }

  return Result{func(std::move(storage_.error))};
}

template <typename V, typename E>
template <typename U, typename std::enable_if<std::is_constructible<V, U>::value, int>::type>
inline V expected<V, E>::value_or(U&& fallback) & {
  if (has_value_) {
    return storage_.value;
  }

  return V{std::forward<U>(fallback)};
}

template <typename V, typename E>
template <typename U, typename std::enable_if<std::is_constructible<V, U>::value, int>::type>
inline V expected<V, E>::value_or(U&& fallback) && {
  if (has_value_) {
    return std::move(storage_.value);
  }

  return V{std::forward<U>(fallback)};
}

template <typename V, typename E>
inline expected<V, E>& expected<V, E>::operator=(const expected<V, E>& rhs) noexcept {
  if (this == &rhs) {
    return *this;
  }

  destroy();

  has_value_ = rhs.has_value_;
  if (has_value_) {
    new (&storage_.value) V{rhs.storage_.value};
  } else {
    new (&storage_.error) E{rhs.storage_.error};
  }

  return *this;
}

template <typename V, typename E>
inline expected<V, E>& expected<V, E>::operator=(expected<V, E>&& rhs) noexcept {
  if (this == &rhs) {
    return *this;
  }

  destroy();

  has_value_ = rhs.has_value_;
  if (has_value_) {
    new (&storage_.value) V{std::move(rhs.storage_.value)};
  } else {
    new (&storage_.error) E{std::move(rhs.storage_.error)};
  }

  return *this;
}

template <typename V, typename E>
inline expected<V, E>::operator bool() const {
  return has_value_;
}

template <typename V, typename E>
inline const V& expected<V, E>::operator*() const& {
  return storage_.value;
}

template <typename V, typename E>
inline V& expected<V, E>::operator*() & {
  return storage_.value;
}

template <typename V, typename E>
inline const V* expected<V, E>::operator->() const& {
  return &storage_.value;
}

template <typename V, typename E>
inline V* expected<V, E>::operator->() & {
  return &storage_.value;
}
