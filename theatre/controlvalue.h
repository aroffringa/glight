#ifndef THEATRE_CONTROLVALUE_H_
#define THEATRE_CONTROLVALUE_H_

#include "mixstyle.h"

#include <cmath>
#include <cstdint>

namespace glight::theatre {

/**
 * @author Andre Offringa
 */
class ControlValue {
 public:
  constexpr ControlValue() : _value(0) {}
  constexpr explicit ControlValue(unsigned value) : _value(value) {}

  constexpr explicit operator bool() const { return _value != 0; }

  constexpr unsigned int UInt() const { return _value; }

  constexpr static ControlValue Zero() { return ControlValue(0); }
  constexpr static ControlValue Max() { return ControlValue((1 << 24) - 1); }
  constexpr static ControlValue FromRatio(double ratio) {
    return ControlValue(static_cast<unsigned>(ratio * MaxUInt()));
  }
  constexpr static unsigned MaxUInt() { return (1 << 24) - 1; }

  constexpr static unsigned Invert(unsigned value) { return MaxUInt() - value; }
  constexpr static unsigned CharToValue(unsigned char value) {
    return (static_cast<unsigned>(value) * MaxUInt()) >> 8;
  }

  static unsigned Mix(unsigned firstValue, unsigned secondValue,
                      MixStyle mixStyle) {
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

  constexpr static unsigned MultiplyValues(unsigned first, unsigned second) {
    if (first >= MaxUInt() && second >= MaxUInt()) return MaxUInt();
    first >>= 9;
    second >>= 9;
    return (first * second) >> 6;
  }

  constexpr static unsigned Fraction(unsigned numerator, unsigned denominator) {
    if (denominator == 0) {
      return numerator == 0 ? 0 : MaxUInt();
    } else {
      const uint64_t n =
          (static_cast<uint64_t>(numerator) << 24u);  // to 48 bits
      const uint64_t d = denominator;                 // remain 24 bits
      return std::min(MaxUInt(),
                      static_cast<unsigned>(n / d));  // from 48 bit to 24 bit
    }
  }

  constexpr static MixStyle CombineMixStyles(MixStyle primaryStyle,
                                             MixStyle secondaryStyle) {
    if (primaryStyle == MixStyle::Default)
      return secondaryStyle;
    else
      return primaryStyle;
  }
  void Set(unsigned int uintValue) { _value = uintValue; }
  constexpr double Ratio() const {
    return (double)_value / (double)((1 << 24) - 1);
  }
  constexpr double RoundedPercentage() const {
    return std::round(1000.0 * (double)_value / (double)((1 << 24) - 1)) * 0.1;
  }

 private:
  unsigned int _value;
};

inline constexpr bool operator==(const ControlValue& lhs,
                                 const ControlValue& rhs) {
  return lhs.UInt() == rhs.UInt();
}

inline constexpr ControlValue operator*(const ControlValue& lhs,
                                        const ControlValue& rhs) {
  return ControlValue(ControlValue::MultiplyValues(lhs.UInt(), rhs.UInt()));
}

inline constexpr ControlValue operator*(const ControlValue& lhs,
                                        unsigned factor) {
  return ControlValue(lhs.UInt() * factor);
}

/**
 * @param ratio Value between 0 and 1.
 */
inline constexpr ControlValue operator*(const ControlValue& lhs, double ratio) {
  return ControlValue(lhs.UInt() * ratio);
}

inline constexpr ControlValue operator/(const ControlValue& lhs,
                                        unsigned factor) {
  return ControlValue(lhs.UInt() / factor);
}

template <class... Pack>
inline ControlValue Min(const ControlValue& first, const ControlValue& second) {
  return ControlValue(std::min(first.UInt(), second.UInt()));
}

template <class... Pack>
inline ControlValue Min(const ControlValue& first, const ControlValue& second,
                        Pack... third) {
  return ControlValue(std::min(first.UInt(), Min(second, third...).UInt()));
}

inline ControlValue Invert(const ControlValue& v) {
  return ControlValue(ControlValue::MaxUInt() -
                      std::min(v.UInt(), ControlValue::MaxUInt()));
}

template <class... Pack>
inline ControlValue Max(const ControlValue& first, const ControlValue& second) {
  return ControlValue(std::max(first.UInt(), second.UInt()));
}

template <class... Pack>
inline ControlValue Max(const ControlValue& first, const ControlValue& second,
                        Pack... third) {
  return ControlValue(std::max(first.UInt(), Max(second, third...).UInt()));
}

inline ControlValue Mix(const ControlValue& firstValue,
                        const ControlValue& secondValue, MixStyle mixStyle) {
  return ControlValue(
      ControlValue::Mix(firstValue.UInt(), secondValue.UInt(), mixStyle));
}

}  // namespace glight::theatre

#endif
