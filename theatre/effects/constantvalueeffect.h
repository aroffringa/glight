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

  virtual EffectType GetType() const override { return EffectType::ConstantValue; }

  unsigned Value() const { return _value; }
  void SetValue(unsigned value) { _value = value; }

 protected:
  virtual void MixImplementation(const std::array<ControlValue, 2> *values, const Timing &timing,
                                 bool primary) override {
    MixToAllOutputs(ControlValue(_value), primary);
  }

 private:
  unsigned _value;
};

}  // namespace glight::theatre

#endif
