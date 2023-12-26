#ifndef GLIGHT_THEATRE_RGB_FILTER_H_
#define GLIGHT_THEATRE_RGB_FILTER_H_

#include <algorithm>

#include "filter.h"

#include "system/optionalnumber.h"

namespace glight::theatre {

/**
 * This filter converts sRGB colour space values into the channels
 * supported by the fixture.
 */
class RgbFilter : public Filter {
 public:
  void Apply(const std::vector<ControlValue>& input,
             std::vector<ControlValue>& output) override {
    unsigned red = input[0].UInt();
    unsigned green = input[1].UInt();
    unsigned blue = input[2].UInt();
    if (ww_index_ && cw_index_) {
      // These equations will mix the cold and warm whites such
      // that they match the requested warmth of the light. This has the
      // consequence that the brightness is much higher when a neutral
      // (RGB) white is requested, and small differences between red and
      // blue have a large effect on the brightness.
      unsigned ww;
      unsigned cw;
      if (red > blue) {
        const unsigned difference = (red - blue) * 64 / 7;
        ww = std::min(red * 57, std::min(green, blue) * 64) / 57;
        cw = std::max(ww, difference) - difference;
      } else {
        const unsigned difference = (blue - red) * 64 / 7;
        cw = std::min(std::min(red, green) * 64, red * 57) / 57;
        ww = std::max(cw, difference) - difference;
      }
      red -= ww / 2 + cw * 57 / 128;
      green -= (ww + cw) * 57 / 128;
      blue -= cw / 2 + ww * 57 / 128;
      output[*ww_index_] = ControlValue(ww);
      output[*cw_index_] = ControlValue(cw);
    } else {
      if (cw_index_) {
        const unsigned rg = std::min(red, green);
        const unsigned cw = std::min(rg * 64, blue * 57) / 64;
        output[*cw_index_] = ControlValue(cw);
      }
      if (ww_index_) {
        const unsigned gb = std::min(green, blue);
        const unsigned ww = std::min(gb * 64, red * 57) / 64;
        output[*ww_index_] = ControlValue(ww);
      }
    }
    if (lime_index_) {
      const unsigned lime = std::min(green, red * 2);
      output[*lime_index_] = ControlValue(lime);
      green -= lime;
      red -= lime / 2;
    }
    if (amber_index_) {
      const unsigned amber = std::min(green * 2, red);
      output[*amber_index_] = ControlValue(amber);
      green -= amber / 2;
      red -= amber;
    }
    if (white_index_) {
      const unsigned white = std::min({green, red, blue});
      output[*white_index_] = ControlValue(white);
      green -= white;
      red -= white;
      blue -= white;
    }
    if (red_index_) {
      output[*red_index_] = ControlValue(red);
    }
    if (green_index_) {
      output[*green_index_] = ControlValue(green);
    }
    if (blue_index_) {
      output[*blue_index_] = ControlValue(blue);
    }
    size_t input_index = 3;
    for (size_t i = 0; i != OutputTypes().size(); ++i) {
      const FunctionType type = OutputTypes()[i];
      switch (type) {
        case FunctionType::Red:
        case FunctionType::Green:
        case FunctionType::Blue:
        case FunctionType::Lime:
        case FunctionType::Amber:
        case FunctionType::White:
        case FunctionType::ColdWhite:
        case FunctionType::WarmWhite:
          // Do nothing
          break;
        default:
          output[i] = input[input_index];
          ++input_index;
          break;
      }
    }
  }

 protected:
  void DetermineInputTypes() override {
    std::vector<FunctionType> input_types{
        FunctionType::Red, FunctionType::Green, FunctionType::Blue};
    for (size_t i = 0; i != OutputTypes().size(); ++i) {
      const FunctionType type = OutputTypes()[i];
      switch (type) {
        case FunctionType::Red:
          red_index_ = i;
          break;
        case FunctionType::Green:
          green_index_ = i;
          break;
        case FunctionType::Blue:
          blue_index_ = i;
          break;
        case FunctionType::Lime:
          lime_index_ = i;
          break;
        case FunctionType::Amber:
          amber_index_ = i;
          break;
        case FunctionType::White:
          white_index_ = i;
          break;
        case FunctionType::ColdWhite:
          cw_index_ = i;
          break;
        case FunctionType::WarmWhite:
          ww_index_ = i;
          break;
        default:
          input_types.emplace_back(type);
          break;
      }
    }
    SetInputTypes(std::move(input_types));
  }

  system::OptionalNumber<size_t> red_index_;
  system::OptionalNumber<size_t> green_index_;
  system::OptionalNumber<size_t> blue_index_;
  system::OptionalNumber<size_t> lime_index_;
  system::OptionalNumber<size_t> amber_index_;
  system::OptionalNumber<size_t> white_index_;
  system::OptionalNumber<size_t> cw_index_;
  system::OptionalNumber<size_t> ww_index_;
};

}  // namespace glight::theatre

#endif
