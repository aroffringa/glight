#ifndef INVERT_EFFECT_H
#define INVERT_EFFECT_H

#include "../effect.h"

#include <string>

class InvertEffect : public Effect {
 public:
  InvertEffect()
      : Effect(2),
        _offThreshold(ControlValue::MaxUInt() * 2 / 100)  // 2 %
  {}

  virtual Effect::Type GetType() const override { return InvertType; }

  unsigned OffThreshold() const { return _offThreshold; }
  void SetOffThreshold(unsigned offThreshold) { _offThreshold = offThreshold; }

 protected:
  virtual void mix(const ControlValue *values,
                   const class Timing &timing) final override {
    unsigned inverted = ControlValue::MaxUInt() - values[0].UInt();
    if (inverted < _offThreshold) inverted = ControlValue::Zero();
    ControlValue value =
        ControlValue::Mix(values[1].UInt(), inverted, ControlValue::Multiply);
    for (const std::pair<Controllable *, size_t> &connection : Connections())
      connection.first->MixInput(connection.second, value);
  }

  virtual FunctionType InputType(size_t inputIndex) const override {
    return inputIndex == 0 ? FunctionType::Effect : FunctionType::Master;
  }

 private:
  unsigned _offThreshold;
};

#endif
