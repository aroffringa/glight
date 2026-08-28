#ifndef THEATRE_MIX_STYLE_H_
#define THEATRE_MIX_STYLE_H_

#include <string>

#include "functiontype.h"

namespace glight::theatre {

enum class MixStyle {
  Default,
  HighestValue,
  Sum,
  LowestValue,
  Multiply,
  First,
  Second,
  LastTakesPrecedence
};

inline std::string ToString(MixStyle mix_style) {
  switch (mix_style) {
    case MixStyle::Default:
    default:
      return "default";
    case MixStyle::HighestValue:
      return "highest_value";
    case MixStyle::Sum:
      return "sum";
    case MixStyle::LowestValue:
      return "lowest_value";
    case MixStyle::Multiply:
      return "multiply";
    case MixStyle::First:
      return "first";
    case MixStyle::Second:
      return "second";
    case MixStyle::LastTakesPrecedence:
      return "last_takes_precedence";
  }
}

inline MixStyle GetMixStyle(const std::string& str) {
  if (str == "highest_value")
    return MixStyle::HighestValue;
  else if (str == "sum")
    return MixStyle::Sum;
  else if (str == "last_takes_precedence")
    return MixStyle::LastTakesPrecedence;
  else if (str == "lowest_value")
    return MixStyle::LowestValue;
  else if (str == "multiply")
    return MixStyle::Multiply;
  else if (str == "first")
    return MixStyle::First;
  else if (str == "second")
    return MixStyle::Second;
  else
    return MixStyle::Default;
}

inline constexpr MixStyle GetMixStyle(FunctionType function_type) {
  switch (function_type) {
    // Positional function types:
    case FunctionType::ColorMacro:
    case FunctionType::ColorTemperature:
    case FunctionType::ColorWheel:
    case FunctionType::Combined:
    case FunctionType::Focus:
    case FunctionType::GoboWheel:
    case FunctionType::Pan:
    case FunctionType::Prism:
    case FunctionType::Saturation:
    case FunctionType::Tilt:
    case FunctionType::Zoom:
    case FunctionType::Unknown:
    // Effects
    case FunctionType::Effect:
    case FunctionType::Hue:
    case FunctionType::Pulse:
    case FunctionType::RotationSpeed:
    case FunctionType::Strobe:
      return MixStyle::LastTakesPrecedence;
    // Colors:
    case FunctionType::Red:
    case FunctionType::Green:
    case FunctionType::Blue:
    case FunctionType::White:
    case FunctionType::Amber:
    case FunctionType::UV:
    case FunctionType::Lime:
    case FunctionType::ColdWhite:
    case FunctionType::WarmWhite:
      // other summed function types:
    case FunctionType::Master:
    case FunctionType::Lightness:
      return MixStyle::Sum;
  }
  return MixStyle::Sum;
}

}  // namespace glight::theatre

#endif
