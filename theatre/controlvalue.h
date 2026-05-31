#ifndef THEATRE_CONTROLVALUE_H_
#define THEATRE_CONTROLVALUE_H_

#include "functiontype.h"
#include "mixstyle.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <sstream>

namespace glight::theatre {

/**
 * @author Andre Offringa
 */
class ControlValue {
 public:
  constexpr ControlValue() noexcept : value_(0) {}
  constexpr explicit ControlValue(uint32_t value) noexcept : value_(value) {}

  constexpr ControlValue(const ControlValue& source) noexcept = default;
  constexpr ControlValue& operator=(const ControlValue& rhs) noexcept = default;

  constexpr explicit operator bool() const noexcept { return value_ != 0; }

  constexpr uint32_t UInt() const noexcept { return value_; }

  constexpr static ControlValue Zero() noexcept { return ControlValue(0); }
  constexpr static ControlValue Max() noexcept {
    return ControlValue((1 << 24) - 1);
  }
  constexpr static ControlValue FromRatio(double ratio) noexcept {
    return ControlValue(
        static_cast<uint32_t>(std::clamp(ratio, 0.0, 1.0) * MaxUInt()));
  }
  constexpr static ControlValue FromUChar(uint8_t value) noexcept {
    return ControlValue(static_cast<uint32_t>(value) * MaxUInt() / 255);
  }
  constexpr static uint32_t MaxUInt() noexcept { return (1 << 24) - 1; }

  constexpr static uint32_t Invert(uint32_t value) noexcept {
    return MaxUInt() - value;
  }
  constexpr static uint32_t CharToValue(uint8_t value) noexcept {
    return (static_cast<uint32_t>(value) * MaxUInt()) / 255;
  }

  static uint32_t Mix(uint32_t firstValue, uint32_t secondValue,
                      MixStyle mixStyle) noexcept {
    switch (mixStyle) {
      default:
      case MixStyle::HighestValue:
        if (firstValue > secondValue)
          return firstValue;
        else
          return secondValue;
      case MixStyle::Default:
      case MixStyle::Sum:
        return firstValue + secondValue;
      case MixStyle::LowestValue:
        if (firstValue > secondValue)
          return secondValue;
        else
          return firstValue;
      case MixStyle::Multiply:
        return MultiplyValues(firstValue, secondValue);
      case MixStyle::First:
        return firstValue;
      case MixStyle::Second:
        return secondValue;
    }
  }

  constexpr static uint32_t MultiplyValues(uint32_t first,
                                           uint32_t second) noexcept {
    if (first >= MaxUInt() && second >= MaxUInt()) return MaxUInt();
    first >>= 9;
    second >>= 9;
    return (first * second) >> 6;
  }

  constexpr static uint32_t Fraction(uint32_t numerator,
                                     uint32_t denominator) noexcept {
    if (denominator == 0) {
      return numerator == 0 ? 0 : MaxUInt();
    } else {
      const uint64_t n =
          (static_cast<uint64_t>(numerator) << 24u);  // to 48 bits
      const uint64_t d = denominator;                 // remain 24 bits
      return std::min(MaxUInt(),
                      static_cast<uint32_t>(n / d));  // from 48 bit to 24 bit
    }
  }

  constexpr static MixStyle CombineMixStyles(MixStyle primaryStyle,
                                             MixStyle secondaryStyle) noexcept {
    if (primaryStyle == MixStyle::Default)
      return secondaryStyle;
    else
      return primaryStyle;
  }
  constexpr double Ratio() const noexcept {
    return (double)value_ / (double)((1 << 24) - 1);
  }
  constexpr double RoundedPercentage() const noexcept {
    return std::round(1000.0 * (double)value_ / (double)((1 << 24) - 1)) * 0.1;
  }
  constexpr uint8_t ToUChar() const noexcept {
    return std::min(value_, ControlValue::MaxUInt()) >> 16;
  }
  void Set(uint32_t uintValue) noexcept { value_ = uintValue; }
  ControlValue& operator+=(ControlValue value) noexcept {
    value_ += value.UInt();
    return *this;
  }

 private:
  uint32_t value_;
};

inline constexpr bool operator==(const ControlValue& lhs,
                                 const ControlValue& rhs) noexcept {
  return lhs.UInt() == rhs.UInt();
}

inline constexpr ControlValue operator+(const ControlValue& lhs,
                                        const ControlValue& rhs) noexcept {
  return ControlValue(lhs.UInt() + rhs.UInt());
}

inline constexpr ControlValue operator-(const ControlValue& lhs,
                                        const ControlValue& rhs) noexcept {
  return ControlValue(lhs.UInt() - rhs.UInt());
}

inline constexpr ControlValue operator*(const ControlValue& lhs,
                                        const ControlValue& rhs) noexcept {
  return ControlValue(ControlValue::MultiplyValues(lhs.UInt(), rhs.UInt()));
}

inline constexpr ControlValue operator*(const ControlValue& lhs,
                                        uint32_t factor) noexcept {
  return ControlValue(lhs.UInt() * factor);
}

/**
 * @param ratio Value between 0 and 1.
 */
inline constexpr ControlValue operator*(const ControlValue& lhs,
                                        double ratio) noexcept {
  return ControlValue(lhs.UInt() * ratio);
}

inline constexpr ControlValue operator/(const ControlValue& lhs,
                                        uint32_t factor) noexcept {
  return ControlValue(lhs.UInt() / factor);
}

template <class... Pack>
inline ControlValue Min(const ControlValue& first,
                        const ControlValue& second) noexcept {
  return ControlValue(std::min(first.UInt(), second.UInt()));
}

template <class... Pack>
inline ControlValue Min(const ControlValue& first, const ControlValue& second,
                        Pack... third) noexcept {
  return ControlValue(std::min(first.UInt(), Min(second, third...).UInt()));
}

inline ControlValue Invert(const ControlValue& v) noexcept {
  return ControlValue(ControlValue::MaxUInt() -
                      std::min(v.UInt(), ControlValue::MaxUInt()));
}

template <class... Pack>
inline ControlValue Max(const ControlValue& first,
                        const ControlValue& second) noexcept {
  return ControlValue(std::max(first.UInt(), second.UInt()));
}

template <class... Pack>
inline ControlValue Max(const ControlValue& first, const ControlValue& second,
                        Pack... third) noexcept {
  return ControlValue(std::max(first.UInt(), Max(second, third...).UInt()));
}

inline ControlValue Mix(ControlValue firstValue, ControlValue secondValue,
                        MixStyle mixStyle) noexcept {
  return ControlValue(
      ControlValue::Mix(firstValue.UInt(), secondValue.UInt(), mixStyle));
}

inline ControlValue MixInput(ControlValue input, ControlValue mix,
                             ControlValue previous_mix,
                             FunctionType function_type) noexcept {
  const MixStyle style = GetMixStyle(function_type);
  if (style == MixStyle::LastTakesPrecedence) {
    if (mix != previous_mix)
      return mix;
    else
      return input;
  } else {
    return ControlValue(ControlValue::Mix(input.UInt(), mix.UInt(), style));
  }
}

inline std::string ToString(const ControlValue& value) {
  std::ostringstream str;
  str << value.UInt() << " (" << std::round(value.Ratio() * 100.0) << "%)";
  return str.str();
}

}  // namespace glight::theatre

#endif
