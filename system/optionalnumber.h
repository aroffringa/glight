#ifndef AOCOMMON_OPTIONAL_NUMBER_H_
#define AOCOMMON_OPTIONAL_NUMBER_H_

#include <cassert>
#include <limits>
#include <optional>
#include <type_traits>

namespace glight::system {

template <typename NumberType>
class OptionalNumber {
 public:
  static NumberType constexpr UnsetValue =
      std::is_signed_v<NumberType> ? std::numeric_limits<NumberType>::min()
                                   : std::numeric_limits<NumberType>::max();

  constexpr OptionalNumber() noexcept = default;

  constexpr explicit OptionalNumber(std::nullopt_t) noexcept {}

  template <class T = NumberType>
  constexpr explicit OptionalNumber(T&& number) noexcept : number_(number) {}

  template <typename T>
  constexpr explicit OptionalNumber(const OptionalNumber<T>& source) noexcept
      : number_(source.number_) {}

  template <typename T>
  constexpr explicit OptionalNumber(const std::optional<T>& source) noexcept
      : number_(source ? *source : UnsetValue) {}

  constexpr OptionalNumber<NumberType> operator=(std::nullopt_t) noexcept {
    number_ = UnsetValue;
    return *this;
  }

  template <class T>
  constexpr OptionalNumber<NumberType> operator=(T&& number) noexcept {
    number_ = number;
    return *this;
  }

  template <class T = NumberType>
  constexpr OptionalNumber<NumberType> operator=(
      const OptionalNumber<T>& rhs) noexcept {
    number_ = rhs.number_;
    return *this;
  }

  template <class T = NumberType>
  constexpr OptionalNumber<NumberType> operator=(
      const std::optional<T>& rhs) noexcept {
    number_ = rhs.number_;
    return *this;
  }

  constexpr const NumberType& operator*() const noexcept {
    assert(HasValue());
    return number_;
  }

  constexpr NumberType& operator*() noexcept {
    assert(HasValue());
    return number_;
  }

  constexpr bool HasValue() const noexcept { return number_ != UnsetValue; }
  constexpr explicit operator bool() const noexcept {
    return number_ != UnsetValue;
  }

  constexpr NumberType Value() const noexcept { return number_; }
  constexpr NumberType ValueOr(NumberType otherwise) const noexcept {
    return number_ == UnsetValue ? otherwise : number_;
  }

  template <typename T>
  constexpr bool operator==(const OptionalNumber<T>& rhs) const {
    return number_ == rhs.number_;
  }

  constexpr bool operator==(NumberType rhs) const { return number_ == rhs; }
  template <typename T>
  constexpr bool operator!=(const OptionalNumber<T>& rhs) const {
    return number_ != rhs.number_;
  }

  constexpr bool operator!=(NumberType rhs) const { return number_ != rhs; }
  template <typename T>
  constexpr bool operator<(const OptionalNumber<T>& rhs) const {
    return rhs.HasValue() && (!HasValue() || number_ < rhs.number_);
  }

  constexpr bool operator<(NumberType rhs) const {
    return !HasValue() || number_ < rhs;
  }

  template <typename T>
  constexpr bool operator>=(const OptionalNumber<T>& rhs) const {
    return !(rhs < *this);
  }

  constexpr bool operator>=(NumberType rhs) const {
    return HasValue() && number_ >= rhs;
  }

  template <typename T>
  constexpr bool operator<=(const OptionalNumber<T>& rhs) const {
    return *this == rhs || *this < rhs;
  }

  constexpr bool operator<=(NumberType rhs) const {
    return *this == rhs || *this < rhs;
  }

  template <typename T>
  constexpr bool operator>(const OptionalNumber<T>& rhs) const {
    return !(*this <= rhs);
  }

  constexpr bool operator>(NumberType rhs) const { return !(*this <= rhs); }

 private:
  NumberType number_ = UnsetValue;
};

template <typename NumberType>
void swap(OptionalNumber<NumberType>& first,
          OptionalNumber<NumberType>& second) {
  using std::swap;
  swap(first.number_, second.number_);
}

}  // namespace glight::system

#endif
