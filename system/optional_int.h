#ifndef SYSTEM_OPTIONAL_INT_H_
#define SYSTEM_OPTIONAL_INT_H_

#include <cassert>
#include <limits>
#include <optional>

namespace glight::system {

template <typename T>
class OptionalInt {
 public:
  constexpr OptionalInt() noexcept = default;

  constexpr OptionalInt(std::nullopt_t) noexcept : value_(MagicValue()) {}

  constexpr OptionalInt(const OptionalInt<T>& other) noexcept = default;

  constexpr explicit OptionalInt(std::in_place_t, T value) noexcept
      : value_(value) {
    assert(value != MagicValue());
  }

  constexpr OptionalInt(T value) noexcept : value_(value) {
    assert(value != MagicValue());
  }

  constexpr OptionalInt<T>& operator=(std::nullopt_t) noexcept {
    value_ = MagicValue();
    return *this;
  }

  constexpr OptionalInt<T>& operator=(const OptionalInt<T>& other) noexcept {
    value_ = other.value_;
    return *this;
  }

  constexpr OptionalInt<T>& operator=(T value) noexcept {
    value_ = value;
    assert(value != MagicValue());
    return *this;
  }

  constexpr const T* operator->() const noexcept { return &value_; }

  constexpr T* operator->() noexcept { return &value_; }

  constexpr const T& operator*() const noexcept { return value_; }

  constexpr T& operator*() noexcept { return value_; }

  constexpr explicit operator bool() const noexcept {
    return value_ != MagicValue();
  }

  constexpr bool has_value() const noexcept { return value_ != MagicValue(); }

  constexpr T& value() {
    if (!has_value())
      throw std::bad_optional_access();
    else
      return value_;
  }

  constexpr const T& value() const {
    if (!has_value())
      throw std::bad_optional_access();
    else
      return value_;
  }

  constexpr T value_or(T default_value) const {
    return has_value() ? value_ : default_value;
  }

  constexpr void swap(OptionalInt<T>& other) {
    std::swap(value_, other.value_);
  }

  constexpr void reset() noexcept { value_ = MagicValue(); }

  constexpr T& emplace(T value) { value_ = value; }

  template <class U>
  friend constexpr bool operator==(const OptionalInt<T>& lhs,
                                   const OptionalInt<U>& rhs) {
    return lhs.value_ == rhs.value_;
  }

  template <class U>
  friend constexpr bool operator!=(const OptionalInt<T>& lhs,
                                   const OptionalInt<U>& rhs) {
    return lhs.value_ != rhs.value_;
  }

  template <class U>
  friend constexpr bool operator<(const OptionalInt<T>& lhs,
                                  const OptionalInt<U>& rhs) {
    if (!rhs)
      return false;
    else if (!lhs)
      return true;
    else
      return lhs.value_ < rhs.value_;
  }

  template <class U>
  friend constexpr bool operator<=(const OptionalInt<T>& lhs,
                                   const OptionalInt<U>& rhs) {
    return lhs == rhs || lhs < rhs;
  }

  template <class U>
  friend constexpr bool operator>(const OptionalInt<T>& lhs,
                                  const OptionalInt<U>& rhs) {
    return rhs < lhs;
  }

  template <class U>
  friend constexpr bool operator>=(const OptionalInt<T>& lhs,
                                   const OptionalInt<U>& rhs) {
    return lhs == rhs || rhs < lhs;
  }

 private:
  static constexpr T MagicValue() { return std::numeric_limits<T>::max(); }

  T value_ = MagicValue();
};

}  // namespace glight::system

#endif
