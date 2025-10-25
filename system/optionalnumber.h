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
class OptionalNumber {
 public:
  static NumberType constexpr UnsetValue =
      std::is_signed_v<NumberType> ? std::numeric_limits<NumberType>::lowest()
                                   : std::numeric_limits<NumberType>::max();

  constexpr OptionalNumber() noexcept = default;

  constexpr explicit OptionalNumber(std::nullopt_t) noexcept {}

  template <class T = NumberType>
  requires(!std::is_same_v<
           T, OptionalNumber<
                  T>>) constexpr explicit OptionalNumber(T number) noexcept
      : number_(number) {}

  constexpr OptionalNumber(const OptionalNumber<NumberType>& source) noexcept =
      default;

  constexpr OptionalNumber(OptionalNumber<NumberType>&& source) noexcept =
      default;

  template <typename T>
  constexpr explicit OptionalNumber(const std::optional<T>& source) noexcept
      : number_(source ? *source : UnsetValue) {}

  constexpr OptionalNumber<NumberType>& operator=(std::nullopt_t) noexcept {
    number_ = UnsetValue;
    return *this;
  }

  template <class T>
  constexpr OptionalNumber<NumberType>& operator=(T number) noexcept {
    number_ = number;
    return *this;
  }

  constexpr OptionalNumber<NumberType>& operator=(
      const OptionalNumber<NumberType>& rhs) noexcept = default;

  constexpr OptionalNumber<NumberType>& operator=(
      OptionalNumber<NumberType>&& rhs) noexcept = default;

  template <class T = NumberType>
  constexpr OptionalNumber<NumberType>& operator=(
      const std::optional<T>& rhs) noexcept {
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
  constexpr explicit operator bool() const noexcept {
    return number_ != UnsetValue;
  }

  constexpr NumberType Value() const noexcept { return number_; }
  constexpr NumberType ValueOr(NumberType otherwise) const noexcept {
    return number_ == UnsetValue ? otherwise : number_;
  }

  template <typename T>
  constexpr bool operator==(const OptionalNumber<T>& rhs) const noexcept {
    return number_ == rhs.number_;
  }

  template <typename T>
  constexpr bool operator!=(const OptionalNumber<T>& rhs) const noexcept {
    return number_ != rhs.number_;
  }

  template <typename T>
  constexpr bool operator<(const OptionalNumber<T>& rhs) const noexcept {
    // When the number is signed, the value for Unset is the lowest possible
    // value, and thus we can directly compare the numbers. Otherwise, a bit
    // more work is required to guarantee correct ordering.
    if constexpr (std::is_signed_v<NumberType>)
      return number_ < rhs.number_;
    else
      return rhs.HasValue() && (!HasValue() || number_ < rhs.number_);
  }

  template <typename T>
  constexpr bool operator>=(const OptionalNumber<T>& rhs) const noexcept {
    return !(*this < rhs);
  }

  template <typename T>
  constexpr bool operator<=(const OptionalNumber<T>& rhs) const noexcept {
    if constexpr (std::is_signed_v<NumberType>)
      return number_ <= rhs.number_;
    else
      return *this == rhs || *this < rhs;
  }

  template <typename T>
  constexpr bool operator>(const OptionalNumber<T>& rhs) const noexcept {
    return !(*this <= rhs);
  }

  constexpr bool operator==(NumberType rhs) const noexcept {
    return number_ == rhs;
  }

  constexpr bool operator!=(NumberType rhs) const noexcept {
    return number_ != rhs;
  }

  constexpr bool operator<(NumberType rhs) const noexcept {
    if constexpr (std::is_signed_v<NumberType>)
      return number_ < rhs;
    else
      return !HasValue() || number_ < rhs;
  }

  constexpr bool operator>=(NumberType rhs) const noexcept {
    if constexpr (std::is_signed_v<NumberType>)
      return number_ >= rhs;
    else
      return HasValue() && number_ >= rhs;
  }

  constexpr bool operator<=(NumberType rhs) const noexcept {
    if constexpr (std::is_signed_v<NumberType>)
      return number_ <= rhs;
    else
      return !HasValue() || number_ <= rhs;
  }

  constexpr bool operator>(NumberType rhs) const noexcept {
    if constexpr (std::is_signed_v<NumberType>)
      return number_ > rhs;
    else
      return HasValue() && number_ > rhs;
  }

  constexpr void Reset() noexcept { number_ = UnsetValue; }

  constexpr void Swap(OptionalNumber<NumberType>& other) {
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
