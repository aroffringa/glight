#include "hue_saturation_lightness_effect.h"

#include "../colordeduction.h"

#include <hsluv.h>

namespace glight::theatre {

using theatre::ControlValue;

namespace {
std::vector<double> MakeTable() {
  double pr = 0;
  double pg = 0;
  double pb = 0;
  double sum_distance = 0.0;
  HslToRgb(0, 1.0, 0.5, pr, pg, pb);
  std::vector<double> table;
  for (size_t h = 0; h != 361; ++h) {
    double r;
    double g;
    double b;
    HslToRgb(h, 1.0, 0.5, r, g, b);
    const double distance = ColorDistance(r, g, b, pr, pg, pb);
    sum_distance += distance;
    table.emplace_back(sum_distance);
    pr = r;
    pg = g;
    pb = b;
  }
  for (double &f : table) f = f * 360.0 / sum_distance;
  return table;
}

std::vector<double> MakeInverseTable(const std::vector<double> &table) {
  std::vector<double> inverse_table;
  inverse_table.reserve(361);
  size_t h_table = 0;
  for (size_t h_star = 0; h_star != 361; ++h_star) {
    while (table[h_table + 1] < h_star && h_table < 360) ++h_table;
    const double h_star_low = table[h_table];
    const double h_star_high = table[h_table + 1];
    const double h =
        h_table + (h_star - h_star_low) / (h_star_high - h_star_low);
    inverse_table.emplace_back(h);
  }
  return inverse_table;
}

}  // namespace

const std::vector<double> HueSaturationLightnessEffect::table_ = MakeTable();
const std::vector<double> HueSaturationLightnessEffect::inverted_table_ =
    MakeInverseTable(table_);

std::array<ControlValue, 3> HueSaturationLightnessEffect::Convert(
    ControlValue h, ControlValue s, ControlValue l) {
  switch (color_space_) {
    case HslColorSpace::LinearHsl: {
      double r;
      double g;
      double b;
      HslToRgb(360.0 * h.Ratio(), s.Ratio(), l.Ratio(), r, g, b);
      return std::array<ControlValue, 3>{ControlValue::FromRatio(r / 255.0),
                                         ControlValue::FromRatio(g / 255.0),
                                         ControlValue::FromRatio(b / 255.0)};
    }
    case HslColorSpace::HslUv: {
      double r;
      double g;
      double b;
      hsluv2rgb(360.0 * h.Ratio(), 100.0 * s.Ratio(), 100.0 * l.Ratio(), &r, &g,
                &b);
      return std::array<ControlValue, 3>{ControlValue::FromRatio(r),
                                         ControlValue::FromRatio(g),
                                         ControlValue::FromRatio(b)};
    }
    default:
    case HslColorSpace::CorrectedHsl: {
      double r;
      double g;
      double b;
      const double h_deg = std::clamp(0.0, 360.0 * h.Ratio(), 360.0);
      const double h_index_lo = std::floor(h_deg);
      const double h_lo = inverted_table_[h_index_lo];
      const double h_hi = inverted_table_[std::ceil(h_deg)];
      const double h = h_lo + (h_deg - h_index_lo) * (h_hi - h_lo);

      HslToRgb(h, s.Ratio(), l.Ratio(), r, g, b);
      return std::array<ControlValue, 3>{ControlValue::FromRatio(r / 255.0),
                                         ControlValue::FromRatio(g / 255.0),
                                         ControlValue::FromRatio(b / 255.0)};
    }
  }
}

void HueSaturationLightnessEffect::mix(const ControlValue *values,
                                       const Timing & /*timing*/, bool /*primary*/) {
  // TODO cache
  std::array<ControlValue, 3> rgb = Convert(values[0], values[1], values[2]);
  for (const std::pair<Controllable *, size_t> &connection : Connections()) {
    switch (connection.first->InputType(connection.second)) {
      case FunctionType::Red:
        connection.first->MixInput(connection.second, rgb[0]);
        break;
      case FunctionType::Green:
        connection.first->MixInput(connection.second, rgb[1]);
        break;
      case FunctionType::Blue:
        connection.first->MixInput(connection.second, rgb[2]);
        break;
      case FunctionType::White:
        connection.first->MixInput(connection.second,
                                   DeduceWhite(rgb[0], rgb[1], rgb[2]));
        break;
      case FunctionType::Amber:
        connection.first->MixInput(connection.second,
                                   DeduceAmber(rgb[0], rgb[1], rgb[2]));
        break;
      case FunctionType::UV:
        connection.first->MixInput(connection.second,
                                   DeduceUv(rgb[0], rgb[1], rgb[2]));
        break;
      case FunctionType::Lime:
        connection.first->MixInput(connection.second,
                                   DeduceLime(rgb[0], rgb[1], rgb[2]));
        break;
      case FunctionType::ColdWhite:
        connection.first->MixInput(connection.second,
                                   DeduceColdWhite(rgb[0], rgb[1], rgb[2]));
        break;
      case FunctionType::WarmWhite:
        connection.first->MixInput(connection.second,
                                   DeduceWarmWhite(rgb[0], rgb[1], rgb[2]));
        break;
      case FunctionType::Hue:
        connection.first->MixInput(connection.second, values[0]);
        break;
      case FunctionType::Saturation:
        connection.first->MixInput(connection.second, values[1]);
        break;
      case FunctionType::Lightness:
        connection.first->MixInput(connection.second, values[2]);
        break;
      case FunctionType::Master:
      case FunctionType::ColorMacro:
      case FunctionType::ColorTemperature:
      case FunctionType::Strobe:
      case FunctionType::Pulse:
      case FunctionType::Rotation:
      case FunctionType::Unknown:
      case FunctionType::Pan:
      case FunctionType::Tilt:
      case FunctionType::Zoom:
      case FunctionType::Effect:
        break;
    }
  }
}

}  // namespace glight::theatre
