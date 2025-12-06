#ifndef THEATRE_EFFECT_TYPE_H_
#define THEATRE_EFFECT_TYPE_H_

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace glight::theatre {

enum class EffectType {
  AudioLevel,
  ColorControl,
  ColorTemperature,
  ConstantValue,
  Curve,
  Delay,
  Dispenser,
  Fade,
  Flicker,
  FluorescentStart,
  FunctionGenerator,
  HueSaturationLightness,
  Invert,
  MusicActivation,
  Pulse,
  RandomSelect,
  RgbMaster,
  Threshold,
  Timer,
  Twinkle,
  Variable
};

inline std::string EffectTypeToName(EffectType type) {
  using ET = EffectType;
  switch (type) {
    case ET::AudioLevel:
      return "Audiolevel";
    case ET::ColorControl:
      return "Color control";
    case ET::ColorTemperature:
      return "Color temperature";
    case ET::ConstantValue:
      return "Constant value";
    case ET::Curve:
      return "Curve";
    case ET::Delay:
      return "Delay";
    case ET::Dispenser:
      return "Dispenser";
    case ET::Fade:
      return "Fade";
    case ET::Flicker:
      return "Flicker";
    case ET::FluorescentStart:
      return "Fluorescent start";
    case ET::FunctionGenerator:
      return "Function generator";
    case ET::HueSaturationLightness:
      return "Hue saturation lightness";
    case ET::Invert:
      return "Invert";
    case ET::MusicActivation:
      return "Music activation";
    case ET::Pulse:
      return "Pulse";
    case ET::RandomSelect:
      return "Random select";
    case ET::RgbMaster:
      return "RGB master";
    case ET::Threshold:
      return "Threshold";
    case ET::Timer:
      return "Timer";
    case ET::Twinkle:
      return "Twinkle";
    case ET::Variable:
      return "Variable";
  }
  return std::string();
}

inline EffectType NameToEffectType(const std::string &name) {
  using ET = EffectType;
  if (name == "Audiolevel")
    return ET::AudioLevel;
  else if (name == "Color control")
    return ET::ColorControl;
  else if (name == "Color temperature")
    return ET::ColorTemperature;
  else if (name == "Constant value")
    return ET::ConstantValue;
  else if (name == "Curve")
    return ET::Curve;
  else if (name == "Delay")
    return ET::Delay;
  else if (name == "Dispenser")
    return ET::Dispenser;
  else if (name == "Fade")
    return ET::Fade;
  else if (name == "Flicker")
    return ET::Flicker;
  else if (name == "Fluorescent start")
    return ET::FluorescentStart;
  else if (name == "Function generator")
    return ET::FunctionGenerator;
  else if (name == "Hue saturation lightness")
    return ET::HueSaturationLightness;
  else if (name == "Invert")
    return ET::Invert;
  else if (name == "Music activation")
    return ET::MusicActivation;
  else if (name == "Pulse")
    return ET::Pulse;
  else if (name == "Random select")
    return ET::RandomSelect;
  else if (name == "RGB master")
    return ET::RgbMaster;
  else if (name == "Threshold")
    return ET::Threshold;
  else if (name == "Timer")
    return ET::Timer;
  else if (name == "Twinkle")
    return ET::Twinkle;
  else if (name == "Variable")
    return ET::Variable;
  else
    throw std::runtime_error("Unknown effect type");
}

inline std::vector<EffectType> GetEffectTypes() {
  using ET = EffectType;
  return std::vector<ET>{ET::AudioLevel,
                         ET::ColorControl,
                         ET::ColorTemperature,
                         ET::ConstantValue,
                         ET::Curve,
                         ET::Delay,
                         ET::Dispenser,
                         ET::Fade,
                         ET::Flicker,
                         ET::FluorescentStart,
                         ET::FunctionGenerator,
                         ET::HueSaturationLightness,
                         ET::Invert,
                         ET::MusicActivation,
                         ET::Pulse,
                         ET::RandomSelect,
                         ET::RgbMaster,
                         ET::Threshold,
                         ET::Timer,
                         ET::Twinkle,
                         ET::Variable};
}

}  // namespace glight::theatre

#endif
