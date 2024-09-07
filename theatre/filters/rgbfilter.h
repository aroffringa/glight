#ifndef GLIGHT_THEATRE_RGB_FILTER_H_
#define GLIGHT_THEATRE_RGB_FILTER_H_

#include <algorithm>
#include <cassert>

#include "filter.h"

#include "system/colormap.h"
#include "system/optionalnumber.h"

#include "theatre/colordeduction.h"

namespace glight::theatre {

enum class RgbFilterMode { MaxBrightness, Balanced, Accurate };

/**
 * This filter converts sRGB colour space values into the channels
 * supported by the fixture.
 */
class RgbFilter : public Filter {
 public:
  FilterType GetType() const override { return FilterType::RgbColorspace; }

  void SetMode(RgbFilterMode mode) { mode_ = mode; }

  void Apply(const std::vector<ControlValue>& input,
             std::vector<ControlValue>& output) override {
    unsigned red = input[0].UInt();
    unsigned green = input[1].UInt();
    unsigned blue = input[2].UInt();
    // composed colour factor
    // The is for if there are more channels apart from composed
    // channels (like lime and amber), to leave some power in.
    constexpr unsigned ccf = 2;
    if (macro_index_ && master_index_) {
      const unsigned m = std::max({red, green, blue});
      if (m != 0) {
        red = ControlValue::Fraction(red, m);
        green = ControlValue::Fraction(green, m);
        blue = ControlValue::Fraction(blue, m);
      }
      const Color color(ControlValue(red).ToUChar(),
                        ControlValue(green).ToUChar(),
                        ControlValue(blue).ToUChar());
      const unsigned short index = color_map_.GetIndex(color);
      const std::vector<theatre::ColorRangeParameters::Range>& ranges =
          OutputTypes()[*macro_index_].GetColorRangeParameters().GetRanges();
      assert(index < ranges.size());
      output[*macro_index_] = ControlValue::FromUChar(
          (ranges[index].input_min + ranges[index].input_max) / 2);
      output[*master_index_] = ControlValue(m);
    } else if (macro_index_) {
      const Color color(input[0].ToUChar(), input[1].ToUChar(),
                        input[2].ToUChar());
      const unsigned short index = color_map_.GetIndex(color);
      const std::vector<theatre::ColorRangeParameters::Range>& ranges =
          OutputTypes()[*macro_index_].GetColorRangeParameters().GetRanges();
      assert(index < ranges.size());
      output[*macro_index_] = ControlValue::FromUChar(
          (ranges[index].input_min + ranges[index].input_max) / 2);
    } else if (cw_index_ && ww_index_ && amber_index_) {
      unsigned ww;
      unsigned cw;
      unsigned amber;
      if (red > blue) {
        const unsigned difference = (red - blue) * 64 / 7;
        ww = std::min(red * 57, std::min(green, blue) * 64) / 57;
        cw = std::max(ww, difference) - difference;
        amber = std::min(red * 32, green * 64) / 32;
      } else {
        const unsigned difference = (blue - red) * 64 / 7;
        cw = std::min(std::min(red, green) * 64, blue * 57) / 57;
        ww = std::max(cw, difference) - difference;
        amber = std::max(cw, difference * 2) - difference * 2;
      }
      output[*cw_index_] = ControlValue(cw);
      output[*ww_index_] = ControlValue(ww);
      output[*amber_index_] = ControlValue(amber);
      red -= (ww + cw * 57 / 64 + amber) / (3 * ccf);
      green -= ((ww + cw) * 57 / 64 + amber / 2) / (3 * ccf);
      blue -= (cw + ww * 57 / 64) / (3 * ccf);
    } else if (ww_index_ && cw_index_) {
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
        cw = std::min(std::min(red, green) * 64, blue * 57) / 57;
        ww = std::max(cw, difference) - difference;
      }
      output[*ww_index_] = ControlValue(ww);
      output[*cw_index_] = ControlValue(cw);
      red -= (ww + cw * 57 / 64) / (2 * ccf);
      green -= ((ww + cw) * 57 / 64) / (2 * ccf);
      blue -= (cw + ww * 57 / 64) / (2 * ccf);
    } else {
      if (cw_index_) {
        const unsigned rg = std::min(red, green);
        const unsigned cw = std::min(rg * 64, blue * 57) / 64;
        output[*cw_index_] = ControlValue(cw);
        red -= cw * 57 / (64 * ccf);
        green -= cw * 57 / (64 * ccf);
        blue -= cw / ccf;
      }
      if (ww_index_) {
        const unsigned gb = std::min(green, blue);
        const unsigned ww = std::min(gb * 64, red * 57) / 64;
        output[*ww_index_] = ControlValue(ww);
        red -= ww / ccf;
        green -= ww * 57 / (64 * ccf);
        blue -= ww * 57 / (64 * ccf);
      }
      if (amber_index_) {
        const unsigned amber = std::min(green * 2, red);
        output[*amber_index_] = ControlValue(amber);
        red -= amber / ccf;
        green -= amber / (2 * ccf);
      }
    }
    if (lime_index_) {
      const unsigned lime = std::min(green, red * 2);
      output[*lime_index_] = ControlValue(lime);
      red -= lime / (2 * ccf);
      green -= lime / ccf;
    }
    if (white_index_) {
      const unsigned white = std::min({green, red, blue});
      output[*white_index_] = ControlValue(white);
      green -= white / ccf;
      red -= white / ccf;
      blue -= white / ccf;
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
      const FunctionType type = OutputTypes()[i].Type();
      switch (type) {
        case FunctionType::Red:
        case FunctionType::Green:
        case FunctionType::Blue:
        case FunctionType::Lime:
        case FunctionType::Amber:
        case FunctionType::White:
        case FunctionType::ColdWhite:
        case FunctionType::WarmWhite:
        case FunctionType::ColorMacro:
        case FunctionType::ColorWheel:
          // Do nothing
          break;
        case FunctionType::Master:
          if (!macro_index_) {
            output[i] = input[input_index];
            ++input_index;
          }
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
    red_index_.Reset();
    green_index_.Reset();
    blue_index_.Reset();
    lime_index_.Reset();
    amber_index_.Reset();
    white_index_.Reset();
    cw_index_.Reset();
    ww_index_.Reset();
    macro_index_.Reset();
    master_index_.Reset();

    std::vector<FixtureTypeFunction> input_types{
        FixtureTypeFunction(FunctionType::Red, 0, {}, 0),
        FixtureTypeFunction(FunctionType::Green, 0, {}, 0),
        FixtureTypeFunction(FunctionType::Blue, 0, {}, 0)};
    for (size_t i = 0; i != OutputTypes().size(); ++i) {
      const FunctionType type = OutputTypes()[i].Type();
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
        case FunctionType::Master:
          master_index_ = i;
          input_types.emplace_back(OutputTypes()[i]);
          break;
        case FunctionType::ColorMacro:
        case FunctionType::ColorWheel:
          macro_index_ = i;
          break;
        default:
          input_types.emplace_back(OutputTypes()[i]);
          break;
      }
    }
    if (macro_index_) {
      // Macro is only used if there is no other means to set the color.
      if (red_index_ || green_index_ || blue_index_ || lime_index_ ||
          amber_index_ || white_index_ || cw_index_ || ww_index_) {
        macro_index_.Reset();
        master_index_.Reset();
      } else {
        const std::vector<ColorRangeParameters::Range>& ranges =
            OutputTypes()[*macro_index_].GetColorRangeParameters().GetRanges();
        std::vector<Color> colors;
        for (const ColorRangeParameters::Range& range : ranges) {
          if (range.color)
            colors.emplace_back(*range.color);
          else
            colors.emplace_back(Color::Black());
        }
        color_map_ = system::ColorMap(colors, 8);
        if (master_index_) {
          for (auto iter = input_types.begin(); iter != input_types.end();
               ++iter) {
            if (iter->Type() == FunctionType::Master) {
              input_types.erase(iter);
              break;
            }
          }
        }
      }
    } else {
      master_index_.Reset();
    }
    SetInputTypes(std::move(input_types));
  }

  RgbFilterMode mode_ = RgbFilterMode::Balanced;

  system::OptionalNumber<size_t> red_index_;
  system::OptionalNumber<size_t> green_index_;
  system::OptionalNumber<size_t> blue_index_;
  system::OptionalNumber<size_t> lime_index_;
  system::OptionalNumber<size_t> amber_index_;
  system::OptionalNumber<size_t> white_index_;
  system::OptionalNumber<size_t> cw_index_;
  system::OptionalNumber<size_t> ww_index_;
  system::OptionalNumber<size_t> macro_index_;
  system::OptionalNumber<size_t> master_index_;
  system::ColorMap color_map_;
};

}  // namespace glight::theatre

#endif
