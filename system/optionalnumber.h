#ifndef AOCOMMON_OPTIONAL_NUMBER_H_
#define AOCOMMON_OPTIONAL_NUMBER_H_

#include <cassert>
#include <limits>
#include <optional>
#include <type_traits>

namespace glight::system {

/**
 * This class behaves like std::optional<T>, with T an integer or floating point
 * type, but implements it using a 'magical number' to represent unset. It
 * therefore does not require more storage to store the state, and because the
 * type is known to be a fundamental type, no (placement) new is required, which
 * allows all methods to be @c noexcept. Because unset is represented by a
 * magical number, comparisons are also faster, as the single value is already
 * strictly ordered.
 *
 * Because of these properties, this class may therefore be more efficient in
 * certain circumstances compared to std::optional.
 *
 * The value that indicates 'unset' is the maximum value for unsigned types and
 * the minimum (most negative) number for signed values, because this is
 * considered the least likely value to conflict with values that the optional
 * number might actually take.
 *
 * Because this class is for situations where performance may matter, the class
 * does not do any runtime checks for cases like overflow, underflow or invalid
 * values.
 *
 * Comparisons behave like std::optional: an unset optional is considered lower
 * than a set optional (independent of the magic value that is used).
 */
template <typename NumberType>
requires(std::is_arithmetic_v<NumberType>) class OptionalNumber {
 public:
  static constexpr NumberType UnsetValue = std::is_signed_v<NumberType>
                                               ? std::numeric_limits<NumberType>::lowest()
                                               : std::numeric_limits<NumberType>::max();

  constexpr OptionalNumber() noexcept = default;

  constexpr explicit OptionalNumber(std::nullopt_t) noexcept {}

  template <class T = NumberType>
  requires(!std::is_same_v<T, OptionalNumber<T>>) constexpr explicit OptionalNumber(
      T number) noexcept
      : number_(number) {
    assert(static_cast<NumberType>(number) != UnsetValue);
  }

  constexpr OptionalNumber(const OptionalNumber<NumberType>& source) noexcept = default;

  constexpr OptionalNumber(OptionalNumber<NumberType>&& source) noexcept = default;

  template <typename T>
  requires std::convertible_to<T, NumberType> constexpr explicit OptionalNumber(
      const std::optional<T>& source) noexcept
      : number_(source ? *source : UnsetValue) {}

  constexpr OptionalNumber<NumberType>& operator=(std::nullopt_t) noexcept {
    number_ = UnsetValue;
    return *this;
  }

  template <class T>
  requires std::convertible_to<T, NumberType> constexpr OptionalNumber<NumberType>& operator=(
      T number) noexcept {
    assert(static_cast<NumberType>(number) != UnsetValue);
    number_ = static_cast<T>(number);
    return *this;
  }

  constexpr OptionalNumber<NumberType>& operator=(const OptionalNumber<NumberType>& rhs) noexcept =
      default;

  constexpr OptionalNumber<NumberType>& operator=(OptionalNumber<NumberType>&& rhs) noexcept =
      default;

  template <class T = NumberType>
  constexpr OptionalNumber<NumberType>& operator=(const std::optional<T>& rhs) noexcept {
    number_ = rhs ? *rhs : UnsetValue;
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
  constexpr explicit operator bool() const noexcept { return number_ != UnsetValue; }

  constexpr NumberType Value() const noexcept {
    assert(HasValue());
    return number_;
  }
  constexpr NumberType ValueOr(NumberType otherwise) const noexcept {
    return number_ == UnsetValue ? otherwise : number_;
  }

  template <typename F>
  constexpr OptionalNumber<NumberType> AndThen(F&& f) const
      noexcept(noexcept(std::forward<F>(f)(number_))) {
    return HasValue() ? std::forward<F>(f)(number_) : OptionalNumber<NumberType>();
  }

  template <typename F>
  constexpr OptionalNumber<NumberType> Transform(F&& f) const
      noexcept(noexcept(std::forward<F>(f)(number_))) {
    return HasValue() ? OptionalNumber<NumberType>(std::forward<F>(f)(number_))
                      : OptionalNumber<NumberType>();
  }

  template <typename F>
  constexpr OptionalNumber<NumberType> OrElse(F&& f) const
      noexcept(noexcept(std::forward<F>(f)(number_))) {
    return HasValue() ? *this : std::forward<F>(f)(number_);
  }

  template <typename T>
  constexpr bool operator==(const OptionalNumber<T>& rhs) const noexcept {
    if constexpr (std::is_same_v<T, NumberType>)
      return number_ == rhs.number_;
    else {
      if (!HasValue() && !rhs.HasValue()) return true;
      if (!HasValue() || !rhs.HasValue()) return false;
      return Value() == rhs.Value();
    }
  }

  template <typename T>
  constexpr bool operator!=(const OptionalNumber<T>& rhs) const noexcept {
    if constexpr (std::is_same_v<T, NumberType>)
      return number_ != rhs.number_;
    else {
      if (HasValue() != rhs.HasValue()) return true;
      return HasValue() && Value() != rhs.Value();
    }
  }

  constexpr bool operator<(const OptionalNumber& rhs) const noexcept {
    // When the number is signed, the value for Unset is the lowest possible
    // value, and thus we can directly compare the numbers. Otherwise, a bit
    // more work is required to guarantee correct ordering.
    if constexpr (std::is_signed_v<NumberType>)
      return number_ < rhs.number_;
    else
      return rhs.HasValue() && (!HasValue() || number_ < rhs.number_);
  }

  constexpr bool operator>=(const OptionalNumber& rhs) const noexcept { return !(*this < rhs); }

  constexpr bool operator<=(const OptionalNumber& rhs) const noexcept {
    if constexpr (std::is_signed_v<NumberType>)
      return number_ <= rhs.number_;
    else
      return *this == rhs || *this < rhs;
  }

  constexpr bool operator>(const OptionalNumber& rhs) const noexcept { return !(*this <= rhs); }

  constexpr friend bool operator==(const OptionalNumber<NumberType>& lhs, NumberType rhs) noexcept {
    return lhs.number_ == rhs;
  }
  constexpr friend bool operator==(NumberType lhs, const OptionalNumber<NumberType>& rhs) noexcept {
    return rhs.number_ == lhs;
  }

  constexpr friend bool operator!=(const OptionalNumber<NumberType>& lhs, NumberType rhs) noexcept {
    return lhs.number_ != rhs;
  }
  constexpr friend bool operator!=(NumberType rhs, const OptionalNumber<NumberType>& lhs) noexcept {
    return rhs.number_ != lhs;
  }

  constexpr friend bool operator<(const OptionalNumber<NumberType>& lhs, NumberType rhs) noexcept {
    if constexpr (std::is_signed_v<NumberType>)
      return lhs.number_ < rhs;
    else
      return !lhs.HasValue() || lhs.number_ < rhs;
  }
  constexpr friend bool operator<(NumberType lhs, const OptionalNumber<NumberType>& rhs) noexcept {
    return rhs > lhs;
  }

  constexpr friend bool operator>=(const OptionalNumber<NumberType>& lhs, NumberType rhs) noexcept {
    if constexpr (std::is_signed_v<NumberType>)
      return lhs.number_ >= rhs;
    else
      return lhs.HasValue() && lhs.number_ >= rhs;
  }
  constexpr friend bool operator>=(NumberType lhs, const OptionalNumber<NumberType>& rhs) noexcept {
    return rhs <= lhs;
  }

  constexpr friend bool operator<=(const OptionalNumber<NumberType>& lhs, NumberType rhs) noexcept {
    if constexpr (std::is_signed_v<NumberType>)
      return lhs.number_ <= rhs;
    else
      return !lhs.HasValue() || lhs.number_ <= rhs;
  }
  constexpr friend bool operator<=(NumberType rhs, const OptionalNumber<NumberType>& lhs) noexcept {
    return rhs >= lhs;
  }

  constexpr friend bool operator>(const OptionalNumber<NumberType>& lhs, NumberType rhs) noexcept {
    if constexpr (std::is_signed_v<NumberType>)
      return lhs.number_ > rhs;
    else
      return lhs.HasValue() && lhs.number_ > rhs;
  }
  constexpr friend bool operator>(NumberType lhs, const OptionalNumber<NumberType>& rhs) noexcept {
    return rhs < lhs;
  }

  constexpr void Reset() noexcept { number_ = UnsetValue; }

  template <typename T = NumberType>
  requires std::convertible_to<T, NumberType> constexpr void Emplace(T value) noexcept {
    assert(static_cast<NumberType>(value) != UnsetValue);
    number_ = static_cast<NumberType>(value);
  }

  constexpr void Swap(OptionalNumber<NumberType>& other) noexcept {
    // To let OptionalNumber also work with types that may implement their own
    // swap function, we bring std::swap in but don't call swap explicitly from
    // the std namespace.
    using std::swap;
    swap(other.number_, number_);
  }

 private:
  NumberType number_ = UnsetValue;
};

template <typename NumberType>
constexpr void swap(OptionalNumber<NumberType>& first,
                    OptionalNumber<NumberType>& second) noexcept {
  first.Swap(second);
}

}  // namespace glight::system

#endif
