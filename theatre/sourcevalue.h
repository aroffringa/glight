#ifndef SOURCE_VALUE_H
#define SOURCE_VALUE_H

#include "presetvalue.h"

class Controllable;

class SourceValue {
 public:
  SourceValue(class Controllable &controllable, size_t inputIndex)
      : _value(controllable, inputIndex),
        _fadeSpeed(0.0),
        _targetValue(_value.Value()) {}

  /**
   * Copy constructor that copies the source but associates it with the given
   * controllable.
   */
  SourceValue(const SourceValue &source, class Controllable &controllable)
      : _value(source._value, controllable),
        _fadeSpeed(0.0),
        _targetValue(source._targetValue) {}

  PresetValue &Preset() { return _value; }

  bool IsIgnorable() const { return _value.IsIgnorable(); }

  class Controllable &Controllable() const {
    return _value.Controllable();
  }

  void ApplyFade(double timePassed) {
    unsigned fadingValue = _value.Value().UInt();
    if (_targetValue != fadingValue) {
      double fadeSpeed = _fadeSpeed;
      if (fadeSpeed == 0.0) {
        fadingValue = _targetValue;
      } else {
        unsigned stepSize = unsigned(std::min<double>(
            timePassed * fadeSpeed * double(ControlValue::MaxUInt() + 1),
            double(ControlValue::MaxUInt() + 1)));
        if (_targetValue > fadingValue) {
          if (fadingValue + stepSize > _targetValue)
            fadingValue = _targetValue;
          else
            fadingValue += stepSize;
        } else {
          if (_targetValue + stepSize > fadingValue)
            fadingValue = _targetValue;
          else
            fadingValue -= stepSize;
        }
      }
      _value.Value().Set(fadingValue);
    }
  }

  void Set(unsigned targetValue, double fadeSpeed) {
    _targetValue = targetValue;
    _fadeSpeed = fadeSpeed;
    if (fadeSpeed == 0.0) {
      _value.SetValue(targetValue);
    }
  }

 private:
  PresetValue _value;
  double _fadeSpeed;
  unsigned _targetValue;
};

#endif
