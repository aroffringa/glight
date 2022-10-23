#ifndef FLICKER_EFFECT_H
#define FLICKER_EFFECT_H

#include "../effect.h"

#include <vector>

namespace glight::theatre {

class FlickerEffect final : public Effect {
 public:
  FlickerEffect()
      : Effect(1),
        _value{{{0u}, {0u}}},
        _previousTime{0.0, 0.0},
        _speed(ControlValue::MaxUInt() / 200.0),
        _independentOutputs(true){};

  virtual EffectType GetType() const override { return EffectType::Flicker; }

  unsigned Speed() const { return _speed; }
  void SetSpeed(unsigned speed) { _speed = speed; }

  bool IndependentOutputs() const { return _independentOutputs; }
  void SetIndependentOutputs(bool independentOutputs) {
    _independentOutputs = independentOutputs;
  }

 private:
  virtual void mix(const ControlValue *values, const Timing &timing,
                   bool primary) override {
    if (values[0]) {
      const size_t count = _independentOutputs ? Connections().size() : 1;
      std::vector<unsigned> &value = _value[primary];
      value.resize(count);
      double delta = 2.0 * (timing.TimeInMS() - _previousTime[primary]) *
                     _speed / ControlValue::MaxUInt();
      _previousTime[primary] = timing.TimeInMS();

      for (size_t i = 0; i != count; ++i) {
        int rnd =
            int(timing.DrawRandomValue()) - int(ControlValue::MaxUInt() / 2);
        int newValue = rnd * delta + int(value[i]);
        if (newValue < 0)
          newValue = 0;
        else if (unsigned(newValue) > ControlValue::MaxUInt())
          newValue = ControlValue::MaxUInt();
        value[i] = newValue;
      }

      if (_independentOutputs) {
        for (size_t i = 0; i != Connections().size(); ++i) {
          Connections()[i].first->MixInput(Connections()[i].second,
                                           values[0] * ControlValue(value[i]));
        }
      } else {
        setConnectedInputs(values[0] * ControlValue(value[0]));
      }
    }
  }

  std::array<std::vector<unsigned>, 2> _value;
  double _previousTime[2];
  /**
   * Speed is in "control value travel per ms"
   */
  unsigned _speed;
  bool _independentOutputs;
};

}  // namespace glight::theatre

#endif
