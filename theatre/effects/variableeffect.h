#ifndef THEATRE_VARIABLE_EFFECT_H_
#define THEATRE_VARIABLE_EFFECT_H_

#include "theatre/effect.h"

namespace glight::theatre {

class VariableEffect final : public Effect {
 public:
  VariableEffect() : Effect(3) {}

  virtual EffectType GetType() const override { return EffectType::Variable; }

  virtual FunctionType InputType(size_t index) const override {
    constexpr FunctionType type[3] = {FunctionType::Red, FunctionType::Green,
                                      FunctionType::Blue};
    return type[index];
  }

  virtual std::vector<Color> InputColors(size_t index) const override {
    switch (index) {
      default:
      case 0:
        return {Color::RedC()};
      case 1:
        return {Color::GreenC()};
      case 2:
        return {Color::BlueC()};
    }
  }

 protected:
  virtual void MixImplementation(const ControlValue *values,
                                 const Timing &timing, bool primary) override {
    for (const std::pair<Controllable *, size_t> &connection : Connections()) {
      const size_t input_index = connection.second;
      switch (connection.first->InputType(input_index)) {
        case FunctionType::Red:
          connection.first->MixInput(input_index, values[0]);
          break;
        case FunctionType::Green:
          connection.first->MixInput(input_index, values[1]);
          break;
        case FunctionType::Blue:
          connection.first->MixInput(input_index, values[2]);
          break;
        default:
          break;
      }
    }
  }

 private:
};

}  // namespace glight::theatre

#endif
