#ifndef THEATRE_VARIABLE_EFFECT_H_
#define THEATRE_VARIABLE_EFFECT_H_

#include "theatre/effect.h"

namespace glight::theatre {

class VariableEffect final : public Effect {
 public:
  VariableEffect() : Effect(3) {}

  virtual EffectType GetType() const override { return EffectType::Variable; }

  virtual FunctionType InputType(size_t index) const override {
    constexpr FunctionType type[3] = {FunctionType::Red, FunctionType::Green, FunctionType::Blue};
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
  virtual void MixImplementation(const ControlValue *values, const Timing &timing,
                                 bool primary) override {
    for (size_t connection_index = 0; connection_index != NConnections(); ++connection_index) {
      const std::pair<const Controllable *, size_t> &connection = GetConnection(connection_index);
      const size_t input_index = connection.second;
      switch (connection.first->InputType(input_index)) {
        case FunctionType::Red:
          MixConnection(connection_index, values[0], primary);
          break;
        case FunctionType::Green:
          MixConnection(connection_index, values[1], primary);
          break;
        case FunctionType::Blue:
          MixConnection(connection_index, values[2], primary);
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
