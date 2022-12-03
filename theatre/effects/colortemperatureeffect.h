#ifndef THEATRE_COLOR_TEMPERATURE_EFFECT_H_
#define THEATRE_COLOR_TEMPERATURE_EFFECT_H_

#include "../effect.h"
#include "../../system/colortemperature.h"

namespace glight::theatre {

class ColorTemperatureEffect final : public Effect {
 public:
  ColorTemperatureEffect() : Effect(1) {}

  EffectType GetType() const override { return EffectType::ColorTemperature; }

  virtual size_t NInputs() const override { return 1; }

  virtual FunctionType InputType(size_t index) const override {
    return FunctionType::ColorTemperature;
  }

  std::vector<Color> InputColors([[maybe_unused]] size_t index) const override {
    return {Color::WarmWhite(), Color::White(), Color::ColdWhite()};
  }

  unsigned MinimumTemperature() const { return min_temperature_; }

  void SetMinimumTemperature(unsigned temperature) {
    min_temperature_ = temperature;
  }

  unsigned MaximumTemperature() const { return max_temperature_; }

  void SetMaximumTemperature(unsigned temperature) {
    max_temperature_ = temperature;
  }

 protected:
  virtual void mix(const ControlValue *values, const Timing &, bool) override {
    const unsigned range =
        std::min(40000u, max_temperature_ - min_temperature_);
    const unsigned scaled_value = values[0].UInt() >> 14;  // make 10 bit
    const unsigned temperature =
        min_temperature_ + ((range * scaled_value) >> 10);
    const theatre::Color rgb = system::TemperatureToRgb(temperature);
    for (const std::pair<Controllable *, size_t> &connection : Connections()) {
      switch (connection.first->InputType(connection.second)) {
        case FunctionType::Red:
          connection.first->MixInput(
              connection.second,
              ControlValue(static_cast<int>(rgb.Red()) << 16));
          break;
        case FunctionType::Green:
          connection.first->MixInput(
              connection.second,
              ControlValue(static_cast<int>(rgb.Green()) << 16));
          break;
        case FunctionType::Blue:
          connection.first->MixInput(
              connection.second,
              ControlValue(static_cast<int>(rgb.Blue()) << 16));
          break;
        case FunctionType::White:
          connection.first->MixInput(connection.second, ControlValue::Max());
          break;
        case FunctionType::Amber:
          // TODO
          break;
        case FunctionType::UV:
          // TODO
          break;
        case FunctionType::Lime:
          // TODO
          break;
        case FunctionType::ColdWhite:
          // TODO
          break;
        case FunctionType::WarmWhite:
          // TODO
          break;
        case FunctionType::Master:
        case FunctionType::ColorTemperature:
        case FunctionType::ColorMacro:
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
  unsigned min_temperature_ = 1500;
  unsigned max_temperature_ = 15000;
};

}  // namespace glight::theatre

#endif
