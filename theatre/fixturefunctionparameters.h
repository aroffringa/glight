#ifndef THEATRE_FIXTURE_FUNCTION_PARAMETERS_H_
#define THEATRE_FIXTURE_FUNCTION_PARAMETERS_H_

#include "color.h"

#include "system/indifferentptr.h"

#include <algorithm>
#include <optional>
#include <vector>

namespace glight::theatre {

namespace details {
template <typename Range>
const Range* GetParameterRange(unsigned input,
                               const std::vector<Range>& ranges) {
  auto iter = std::upper_bound(ranges.begin(), ranges.end(), input,
                               [](unsigned lhs, const Range& rhs) -> bool {
                                 return lhs < rhs.input_max;
                               });
  if (iter == ranges.end() || iter->input_min > input)
    return nullptr;
  else
    return &*iter;
}
}  // namespace details

struct ColorRangeParameters {
  struct Range {
    constexpr Range(unsigned input_min_, unsigned input_max_,
                    const std::optional<Color>& color_)
        : input_min(input_min_), input_max(input_max_), color(color_) {}

    unsigned input_min;
    unsigned input_max;
    std::optional<Color> color;
  };
  std::vector<Range>& GetRanges() { return ranges_; }
  const std::vector<Range>& GetRanges() const { return ranges_; }

  std::optional<Color> GetColor(unsigned input) const {
    const Range* range = details::GetParameterRange(input, ranges_);
    if (!range) {
      return {};
    } else {
      return range->color;
    }
  }

 private:
  // Sorted list that maps input ranges to a (optional) color
  std::vector<Range> ranges_;
};

struct RotationSpeedParameters {
  struct Range {
    constexpr Range(unsigned input_min_, unsigned input_max_, int speed_min_,
                    int speed_max_)
        : input_min(input_min_),
          input_max(input_max_),
          speed_min(speed_min_),
          speed_max(speed_max_) {}

    unsigned input_min;
    unsigned input_max;
    int speed_min;
    int speed_max;
  };
  std::vector<Range>& GetRanges() { return ranges_; }
  const std::vector<Range>& GetRanges() const { return ranges_; }

  int GetSpeed(unsigned input) const {
    const Range* range = details::GetParameterRange(input, ranges_);
    if (!range) {
      return 0;
    } else {
      // linearly interpolate speed over the range
      return int(input - range->input_min) *
                 (range->speed_max - range->speed_min) /
                 int(range->input_max - range->input_min) +
             range->speed_min;
    }
  }

 private:
  // Sorted list that maps input ranges to speed ranges
  std::vector<Range> ranges_;
};

struct FixtureFunctionParameters {
  void SetRotationSpeedParameters(
      const RotationSpeedParameters& rotation_parameters) {
    parameters =
        system::MakeIndifferent<RotationSpeedParameters>(rotation_parameters);
  }
  void UnsetRotationParameters() {
    parameters.Reset<RotationSpeedParameters>();
  }

  void SetColorRangeParameters(const ColorRangeParameters& macro_parameters) {
    parameters =
        system::MakeIndifferent<ColorRangeParameters>(macro_parameters);
  }
  void UnsetColorRangeParameters() { parameters.Reset<ColorRangeParameters>(); }

  system::IndifferentPtr parameters;
};

}  // namespace glight::theatre

#endif
