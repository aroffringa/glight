#ifndef INVERT_EFFECT_H
#define INVERT_EFFECT_H

#include "../effect.h"

#include <string>

namespace glight::theatre {

class InvertEffect final : public Effect {
 public:
  InvertEffect()
      : Effect(2),
        _offThreshold(ControlValue::MaxUInt() * 2 / 100)  // 2 %
  {}

  virtual EffectType GetType() const override { return EffectType::Invert; }

  unsigned OffThreshold() const { return _offThreshold; }
  void SetOffThreshold(unsigned offThreshold) { _offThreshold = offThreshold; }

 protected:
  virtual void mix(const ControlValue *values, const Timing &timing,
                   bool primary) override {
    unsigned inverted = ControlValue::MaxUInt() - values[0].UInt();
    if (inverted < _offThreshold) inverted = 0;
    ControlValue value =
        ControlValue::Mix(values[1].UInt(), inverted, MixStyle::Multiply);
    for (const std::pair<Controllable *, size_t> &connection : Connections())
      connection.first->MixInput(connection.second, value);
  }

  virtual FunctionType InputType(size_t inputIndex) const override {
    return inputIndex == 0 ? FunctionType::Effect : FunctionType::Master;
  }

 private:
  unsigned _offThreshold;
};

}  // namespace glight::theatre

#endif
