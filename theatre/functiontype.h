#ifndef THEATRE_FUNCTION_TYPE_H_
#define THEATRE_FUNCTION_TYPE_H_

#include <stdexcept>
#include <string>
#include <vector>

#include "color.h"

namespace glight::theatre {

enum class FunctionType {
  Master,
  Red,
  Green,
  Blue,
  White,
  Amber,
  UV,
  Lime,
  ColorMacro,
  Strobe,
  Pulse,
  Rotation,
  Pan,
  Tilt,
  Zoom,
  Effect,
  ColdWhite,
  WarmWhite,
  ColorTemperature,
  Hue,
  Saturation,
  Lightness,
  Unknown
};

inline std::vector<FunctionType> GetFunctionTypes() {
  using FT = FunctionType;
  return std::vector<FunctionType>{FT::Master,
                                   FT::Red,
                                   FT::Green,
                                   FT::Blue,
                                   FT::White,
                                   FT::Amber,
                                   FT::UV,
                                   FT::Lime,
                                   FT::ColorMacro,
                                   FT::Strobe,
                                   FT::Pulse,
                                   FT::Rotation,
                                   FT::Pan,
                                   FT::Tilt,
                                   FT::Zoom,
                                   FT::Effect,
                                   FT::ColdWhite,
                                   FT::WarmWhite,
                                   FT::ColorTemperature,
                                   FT::Hue,
                                   FT::Saturation,
                                   FT::Lightness,
                                   FT::Unknown};
}

inline const char* AbbreviatedFunctionType(FunctionType functionType) {
  switch (functionType) {
    case FunctionType::Amber:
      return "A";
    case FunctionType::Blue:
      return "B";
    case FunctionType::ColdWhite:
      return "CW";
    case FunctionType::ColorMacro:
      return "C";
    case FunctionType::ColorTemperature:
      return "T";
    case FunctionType::Effect:
      return "E";
    case FunctionType::Green:
      return "G";
    case FunctionType::Hue:
      return "H";
    case FunctionType::Lime:
      return "L";
    case FunctionType::Lightness:
      return "LI";
    case FunctionType::Master:
      return "M";
    case FunctionType::Red:
      return "R";
    case FunctionType::UV:
      return "U";
    case FunctionType::WarmWhite:
      return "WW";
    case FunctionType::White:
      return "W";
    case FunctionType::Pan:
      return "PN";
    case FunctionType::Pulse:
      return "P";
    case FunctionType::Rotation:
      return "O";
    case FunctionType::Saturation:
      return "SA";
    case FunctionType::Strobe:
      return "S";
    case FunctionType::Tilt:
      return "TL";
    case FunctionType::Zoom:
      return "Z";
    case FunctionType::Unknown:
      return "?";
  }
  return nullptr;
}

inline FunctionType GetFunctionType(const std::string& name) {
  const char c = name.empty() ? 0 : name[0];
  switch (c) {
    case 'A':
      if (name == "Amber") return FunctionType::Amber;
      break;
    case 'B':
      if (name == "Blue") return FunctionType::Blue;
      break;
    case 'C':
      if (name == "Cold white")
        return FunctionType::ColdWhite;
      else if (name == "Color macro")
        return FunctionType::ColorMacro;
      break;
    case 'E':
      if (name == "Effect") return FunctionType::Effect;
      break;
    case 'G':
      if (name == "Green") return FunctionType::Green;
      break;
    case 'H':
      if (name == "Hue") return FunctionType::Hue;
      break;
    case 'L':
      if (name == "Lightness") return FunctionType::Lightness;
      if (name == "Lime") return FunctionType::Lime;
      break;
    case 'M':
      if (name == "Master") return FunctionType::Master;
      break;
    case 'P':
      if (name == "Pan")
        return FunctionType::Pan;
      else if (name == "Pulse")
        return FunctionType::Pulse;
      break;
    case 'R':
      if (name == "Red") return FunctionType::Red;
      if (name == "Rotation") return FunctionType::Rotation;
      break;
    case 'S':
      if (name == "Saturation") return FunctionType::Saturation;
      if (name == "Strobe") return FunctionType::Strobe;
      break;
    case 'T':
      if (name == "Temperature")
        return FunctionType::ColorTemperature;
      else if (name == "Tilt")
        return FunctionType::Tilt;
      break;
    case 'U':
      if (name == "UV") return FunctionType::UV;
      break;
    case 'W':
      if (name == "Warm white")
        return FunctionType::WarmWhite;
      else if (name == "White")
        return FunctionType::White;
      break;
    case 'Z':
      if (name == "Zoom") return FunctionType::Zoom;
      break;
  }
  throw std::runtime_error("Function type not found: " + name);
}

inline std::string ToString(FunctionType functionType) {
  switch (functionType) {
    case FunctionType::Amber:
      return "Amber";
    case FunctionType::Blue:
      return "Blue";
    case FunctionType::ColorTemperature:
      return "Color temperature";
    case FunctionType::ColdWhite:
      return "Cold white";
    case FunctionType::ColorMacro:
      return "Color macro";
    case FunctionType::Effect:
      return "Effect";
    case FunctionType::Green:
      return "Green";
    case FunctionType::Hue:
      return "Hue";
    case FunctionType::Lightness:
      return "Lightness";
    case FunctionType::Lime:
      return "Lime";
    case FunctionType::Master:
      return "Master";
    case FunctionType::Pan:
      return "Pan";
    case FunctionType::Pulse:
      return "Pulse";
    case FunctionType::Red:
      return "Red";
    case FunctionType::Rotation:
      return "Rotation";
    case FunctionType::Saturation:
      return "Saturation";
    case FunctionType::Strobe:
      return "Strobe";
    case FunctionType::Tilt:
      return "Tilt";
    case FunctionType::Unknown:
      return "Unknown";
    case FunctionType::UV:
      return "UV";
    case FunctionType::WarmWhite:
      return "Warm white";
    case FunctionType::White:
      return "White";
    case FunctionType::Zoom:
      return "Zoom";
  }
  return "?";
}

inline constexpr Color GetFunctionColor(FunctionType type) {
  switch (type) {
    case FunctionType::ColorMacro:
    case FunctionType::Effect:
    case FunctionType::Lightness:
    case FunctionType::Master:
    case FunctionType::Pulse:
    case FunctionType::Rotation:
    case FunctionType::Pan:
    case FunctionType::Saturation:
    case FunctionType::Strobe:
    case FunctionType::Tilt:
    case FunctionType::Zoom:
    case FunctionType::Unknown:
      return Color::Black();
    case FunctionType::Red:
    case FunctionType::Hue:
      return Color::RedC();
    case FunctionType::Green:
      return Color::GreenC();
    case FunctionType::Blue:
      return Color::BlueC();
    case FunctionType::White:
      return Color::White();
    case FunctionType::Amber:
      return Color::Amber();
    case FunctionType::UV:
      return Color::UV();
    case FunctionType::Lime:
      return Color::Lime();
    case FunctionType::ColdWhite:
      return Color::ColdWhite();
    case FunctionType::WarmWhite:
      return Color::WarmWhite();
    case FunctionType::ColorTemperature:
      return Color::White();
  }
  return Color::Black();
}

}  // namespace glight::theatre

#endif
