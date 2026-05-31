#ifndef THEATRE_COLOR_CONTROL_EFFECT_H_
#define THEATRE_COLOR_CONTROL_EFFECT_H_

#include "../colordeduction.h"
#include "../effect.h"

namespace glight::theatre {

class ColorControlEffect final : public Effect {
 public:
  ColorControlEffect() : Effect(4) {}

  EffectType GetType() const override { return EffectType::ColorControl; }

  virtual FunctionType InputType(size_t index) const override {
    switch (index) {
      default:
      case 0:
        return FunctionType::Master;
      case 1:
        return FunctionType::Red;
      case 2:
        return FunctionType::Green;
      case 3:
        return FunctionType::Blue;
    }
  }

  std::vector<Color> InputColors([[maybe_unused]] size_t index) const override {
    switch (index) {
      default:
      case 0:
        return {};
      case 1:
        return {Color::RedC()};
      case 2:
        return {Color::GreenC()};
      case 3:
        return {Color::BlueC()};
    }
  }

 protected:
  virtual void MixImplementation(const ControlValue *values, const Timing &timing,
                                 bool primary) override {
    for (size_t connection_index = 0; connection_index != NConnections(); ++connection_index) {
      const std::pair<const Controllable *, size_t> &connection = GetConnection(connection_index);
      switch (connection.first->InputType(connection.second)) {
        case FunctionType::Red: {
          const ControlValue v = values[0] * values[1];
          MixConnection(connection_index, v, primary);
        } break;
        case FunctionType::Green: {
          const ControlValue v = values[0] * values[2];
          MixConnection(connection_index, v, primary);
        } break;
        case FunctionType::Blue: {
          const ControlValue v = values[0] * values[3];
          MixConnection(connection_index, v, primary);
        } break;
        case FunctionType::White: {
          const ControlValue v = values[0] * DeduceWhite(values[1], values[2], values[3]);
          MixConnection(connection_index, v, primary);
        } break;
        case FunctionType::Amber: {
          const ControlValue v = values[0] * DeduceAmber(values[1], values[2], values[3]);
          MixConnection(connection_index, v, primary);
        } break;
        case FunctionType::UV: {
          const ControlValue v = values[0] * DeduceUv(values[1], values[2], values[3]);
          MixConnection(connection_index, v, primary);
        } break;
        case FunctionType::Lime: {
          const ControlValue v = values[0] * DeduceLime(values[1], values[2], values[3]);
          MixConnection(connection_index, v, primary);
        } break;
        case FunctionType::ColdWhite: {
          const ControlValue v = values[0] * DeduceColdWhite(values[1], values[2], values[3]);
          MixConnection(connection_index, v, primary);
        } break;
        case FunctionType::WarmWhite: {
          const ControlValue v = values[0] * DeduceWarmWhite(values[1], values[2], values[3]);
          MixConnection(connection_index, v, primary);
        } break;
        default:
          break;
      }
    }
  }

 private:
};

}  // namespace glight::theatre

#endif
