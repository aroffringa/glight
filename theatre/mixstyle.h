#ifndef THEATRE_MIX_STYLE_H_
#define THEATRE_MIX_STYLE_H_

#include <string>

namespace glight::theatre {

enum class MixStyle {
  Default,
  HighestValue,
  Sum,
  LowestValue,
  Multiply,
  First,
  Second
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
  }
}

inline MixStyle GetMixStyle(const std::string& str) {
  if (str == "highest_value")
    return MixStyle::HighestValue;
  else if (str == "sum")
    return MixStyle::Sum;
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

}  // namespace glight::theatre

#endif
