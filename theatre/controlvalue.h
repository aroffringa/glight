#ifndef THEATRE_CONTROLVALUE_H_
#define THEATRE_CONTROLVALUE_H_

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
  constexpr ControlValue() noexcept : _value(0) {}
  constexpr explicit ControlValue(unsigned value) noexcept : _value(value) {}

  constexpr explicit operator bool() const noexcept { return _value != 0; }

  constexpr unsigned int UInt() const noexcept { return _value; }

  constexpr static ControlValue Zero() noexcept { return ControlValue(0); }
  constexpr static ControlValue Max() noexcept {
    return ControlValue((1 << 24) - 1);
  }
  constexpr static ControlValue FromRatio(double ratio) noexcept {
    return ControlValue(
        static_cast<unsigned>(std::clamp(ratio, 0.0, 1.0) * MaxUInt()));
  }
  constexpr static ControlValue FromUChar(unsigned char value) noexcept {
    return ControlValue(static_cast<unsigned>(value) * MaxUInt() / 255);
  }
  constexpr static unsigned MaxUInt() noexcept { return (1 << 24) - 1; }

  constexpr static unsigned Invert(unsigned value) noexcept {
    return MaxUInt() - value;
  }
  constexpr static unsigned CharToValue(unsigned char value) noexcept {
    return (static_cast<unsigned>(value) * MaxUInt()) / 255;
  }

  static unsigned Mix(unsigned firstValue, unsigned secondValue,
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

  constexpr static unsigned MultiplyValues(unsigned first,
                                           unsigned second) noexcept {
    if (first >= MaxUInt() && second >= MaxUInt()) return MaxUInt();
    first >>= 9;
    second >>= 9;
    return (first * second) >> 6;
  }

  constexpr static unsigned Fraction(unsigned numerator,
                                     unsigned denominator) noexcept {
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
                                             MixStyle secondaryStyle) noexcept {
    if (primaryStyle == MixStyle::Default)
      return secondaryStyle;
    else
      return primaryStyle;
  }
  constexpr double Ratio() const noexcept {
    return (double)_value / (double)((1 << 24) - 1);
  }
  constexpr double RoundedPercentage() const noexcept {
    return std::round(1000.0 * (double)_value / (double)((1 << 24) - 1)) * 0.1;
  }
  constexpr unsigned char ToUChar() const noexcept {
    return std::min(_value, ControlValue::MaxUInt()) >> 16;
  }
  void Set(unsigned int uintValue) noexcept { _value = uintValue; }
  ControlValue& operator+=(ControlValue value) noexcept {
    _value += value.UInt();
    return *this;
  }

 private:
  unsigned int _value;
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
                                        unsigned factor) noexcept {
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
                                        unsigned factor) noexcept {
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

inline ControlValue Mix(const ControlValue& firstValue,
                        const ControlValue& secondValue,
                        MixStyle mixStyle) noexcept {
  return ControlValue(
      ControlValue::Mix(firstValue.UInt(), secondValue.UInt(), mixStyle));
}

inline std::string ToString(const ControlValue& value) {
  std::ostringstream str;
  str << value.UInt() << " (" << std::round(value.Ratio() * 100.0) << "%)";
  return str.str();
}

}  // namespace glight::theatre

#endif
