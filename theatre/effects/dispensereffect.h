#ifndef THEATRE_DISPENSER_EFFECT_H_
#define THEATRE_DISPENSER_EFFECT_H_

#include "../effect.h"

namespace glight::theatre {

class DispenserEffect final : public Effect {
 public:
  DispenserEffect() : Effect(1) {}

  virtual EffectType GetType() const override { return EffectType::Dispenser; }

 protected:
  virtual void mix(const ControlValue *values, const Timing &timing,
                   bool primary) override {
    setConnectedInputs(ControlValue(values[0]));
  }

 private:
};

}  // namespace glight::theatre

#endif
