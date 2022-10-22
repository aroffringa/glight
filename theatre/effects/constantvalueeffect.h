#ifndef CONSTANT_VALUE_EFFECT_H
#define CONSTANT_VALUE_EFFECT_H

#include "../effect.h"

namespace glight::theatre {

class ConstantValueEffect final : public Effect {
 public:
  ConstantValueEffect()
      : Effect(0),
        _value(ControlValue::MaxUInt())  // 2 %
  {}

  virtual Effect::Type GetType() const override { return ConstantValueType; }

  unsigned Value() const { return _value; }
  void SetValue(unsigned value) { _value = value; }

 protected:
  virtual void mix(const ControlValue *values, const Timing &timing,
                   bool primary) override {
    setConnectedInputs(ControlValue(_value));
  }

 private:
  unsigned _value;
};

}  // namespace glight::theatre

#endif
