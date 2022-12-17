#ifndef THEATRE_COLOR_CONTROL_EFFECT_H_
#define THEATRE_COLOR_CONTROL_EFFECT_H_

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
  virtual void mix(const ControlValue *values, const Timing &timing,
                   bool primary) override {
    for (const std::pair<Controllable *, size_t> &connection : Connections()) {
      switch (connection.first->InputType(connection.second)) {
        case FunctionType::Red: {
          const ControlValue v = values[0] * values[1];
          connection.first->MixInput(connection.second, v);
        } break;
        case FunctionType::Green: {
          const ControlValue v = values[0] * values[2];
          connection.first->MixInput(connection.second, v);
        } break;
        case FunctionType::Blue: {
          const ControlValue v = values[0] * values[3];
          connection.first->MixInput(connection.second, v);
        } break;
        case FunctionType::White: {
          const ControlValue white = Min(values[1], values[2], values[3]);
          const ControlValue v = values[0] * white;
          connection.first->MixInput(connection.second, v);
        } break;
        case FunctionType::Amber: {
          const unsigned amber =
              std::min(values[1].UInt() / 2, values[2].UInt()) * 2;
          const ControlValue v = values[0] * ControlValue(amber);
          connection.first->MixInput(connection.second, v);
        } break;
        case FunctionType::UV: {
          const unsigned uv =
              std::min(values[3].UInt() / 3, values[1].UInt()) * 3;
          const ControlValue v = values[0] * ControlValue(uv);
          connection.first->MixInput(connection.second, v);
        } break;
        case FunctionType::Lime: {
          const unsigned lime =
              std::min(values[2].UInt() / 2, values[1].UInt()) * 2;
          const ControlValue v = values[0] * ControlValue(lime);
          connection.first->MixInput(connection.second, v);
        } break;
        case FunctionType::ColdWhite: {
          const unsigned rg = std::min(values[1].UInt(), values[2].UInt());
          const unsigned cw = std::min(rg * 64, values[3].UInt() * 57) / 64;
          const ControlValue v = values[0] * ControlValue(cw);
          connection.first->MixInput(connection.second, v);
        } break;
        case FunctionType::WarmWhite: {
          const unsigned gb = std::min(values[2].UInt(), values[3].UInt());
          const unsigned ww = std::min(gb * 64, values[1].UInt() * 57) / 64;
          const ControlValue v = values[0] * ControlValue(ww);
          connection.first->MixInput(connection.second, v);
        } break;
        case FunctionType::Master:
        case FunctionType::ColorMacro:
        case FunctionType::ColorTemperature:
        case FunctionType::Strobe:
        case FunctionType::Pulse:
        case FunctionType::Rotation:
        case FunctionType::Unknown:
        case FunctionType::Pan:
        case FunctionType::Tilt:
        case FunctionType::Effect:
          break;
      }
    }
  }

 private:
};

}  // namespace glight::theatre

#endif
