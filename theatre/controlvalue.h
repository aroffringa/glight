#ifndef CONTROLVALUE_H
#define CONTROLVALUE_H

#include <cmath>

/**
        @author Andre Offringa
*/
class ControlValue {
public:
  enum MixStyle {
    Default,
    HighestValue,
    Sum,
    LowestValue,
    Multiply,
    First,
    Second
  };

  ControlValue() {}
  ControlValue(unsigned value) : _value(value) {}
  ControlValue(const ControlValue &source) = default;

  operator bool() const { return _value != 0; }

  unsigned int UInt() const { return _value; }

  static ControlValue Zero() { return ControlValue(0); }
  static ControlValue Max() { return ControlValue((1 << 24) - 1); }
  static unsigned MaxUInt() { return (1 << 24) - 1; }

  static unsigned Mix(unsigned firstValue, unsigned secondValue,
                      MixStyle mixStyle) {
    switch (mixStyle) {
    default:
    case HighestValue:
      if (firstValue > secondValue)
        return firstValue;
      else
        return secondValue;
    case Default:
    case Sum:
      return firstValue + secondValue;
    case LowestValue:
      if (firstValue > secondValue)
        return secondValue;
      else
        return firstValue;
    case Multiply:
      return MultiplyValues(firstValue, secondValue);
    case First:
      return firstValue;
    case Second:
      return secondValue;
    }
  }

  static unsigned MultiplyValues(unsigned first, unsigned second) {
    first >>= 9;
    second >>= 9;
    return (first * second) >> 6;
  }

  static MixStyle CombineMixStyles(MixStyle primaryStyle,
                                   MixStyle secondaryStyle) {
    if (primaryStyle == Default)
      return secondaryStyle;
    else
      return primaryStyle;
  }
  void Set(unsigned int uintValue) { _value = uintValue; }
  double Ratio() const { return (double)_value / (double)((1 << 24) - 1); }
  double RoundedPercentage() const {
    return std::round(1000.0 * (double)_value / (double)((1 << 24) - 1)) * 0.1;
  }

private:
  unsigned int _value;
};

typedef ControlValue::MixStyle MixStyle;

#endif
