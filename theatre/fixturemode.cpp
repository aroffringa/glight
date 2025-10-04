#include "fixturemode.h"

#include <array>

#include "fixture.h"
#include "fixturetype.h"

#include "system/colortemperature.h"

namespace glight::theatre {

void FixtureMode::UpdateFunctions() {
  std::array<unsigned, 3> max_values;
  max_values[0] = 0;
  max_values[1] = 0;
  max_values[2] = 0;
  for (const FixtureModeFunction &f : functions_) {
    if (IsColor(f.Type())) {
      const Color c = GetFunctionColor(f.Type());
      max_values[0] += c.Red();
      max_values[1] += c.Green();
      max_values[2] += c.Blue();
    }
  }
  scaling_value_ = std::max({max_values[0], max_values[1], max_values[2], 1U});
}

Color FixtureMode::GetColor(const Fixture &fixture,
                            const ValueSnapshot &snapshot,
                            size_t shapeIndex) const {
  unsigned red = 0;
  unsigned green = 0;
  unsigned blue = 0;
  unsigned master = 255;
  std::optional<Color> macro_color;
  for (size_t i = 0; i != functions_.size(); ++i) {
    const FixtureModeFunction &function = functions_[i];
    if (function.Shape() == shapeIndex) {
      const FixtureFunction &ff = *fixture.Functions()[i];
      // Fine channel is ignored if it is present, because we can't
      // visualize 16-bit rgb values anyway...
      const unsigned channel_value = ff.GetCharValue(snapshot);
      const FunctionType type = function.Type();
      if (type == FunctionType::Master) {
        master = channel_value;
      } else if (type == FunctionType::ColorMacro ||
                 type == FunctionType::ColorWheel) {
        macro_color =
            function.GetColorRangeParameters().GetColor(channel_value);
      } else if (type == FunctionType::ColorTemperature) {
        constexpr unsigned min_temperature = 2800;
        constexpr unsigned max_temperature = 8000;
        const unsigned temperature =
            channel_value * (max_temperature - min_temperature) / 255 +
            min_temperature;
        macro_color = system::TemperatureToRgb(temperature);
      } else if (IsColor(type)) {
        const Color c = GetFunctionColor(function.Type()) * channel_value;
        red += c.Red();
        green += c.Green();
        blue += c.Blue();
      }
    }
  }
  if (macro_color) {
    red = macro_color->Red();
    green = macro_color->Green();
    blue = macro_color->Blue();
    return Color(red * master / 256, green * master / 256, blue * master / 256);
  } else {
    return Color(red * master / scaling_value_, green * master / scaling_value_,
                 blue * master / scaling_value_);
  }
}

int FixtureMode::GetRotationSpeed(const Fixture &fixture,
                                  const ValueSnapshot &snapshot,
                                  size_t shape_index) const {
  for (size_t i = 0; i != functions_.size(); ++i) {
    if (functions_[i].Shape() == shape_index &&
        functions_[i].Type() == FunctionType::RotationSpeed) {
      const unsigned channel_value =
          fixture.Functions()[i]->GetCharValue(snapshot);
      return functions_[i].GetRotationSpeedParameters().GetSpeed(channel_value);
    }
  }
  return 0;
}

double FixtureMode::GetPan(const Fixture &fixture,
                           const ValueSnapshot &snapshot,
                           size_t shape_index) const {
  for (size_t i = 0; i != functions_.size(); ++i) {
    if (functions_[i].Shape() == shape_index &&
        functions_[i].Type() == FunctionType::Pan) {
      const unsigned channel_value =
          fixture.Functions()[i]->GetControlValue(snapshot);
      const double max_pan = Type().MaxPan();
      const double min_pan = Type().MinPan();
      return (max_pan - min_pan) * channel_value / ControlValue::MaxUInt() +
             min_pan;
    }
  }
  return 0.0;
}

double FixtureMode::GetTilt(const Fixture &fixture,
                            const ValueSnapshot &snapshot,
                            size_t shape_index) const {
  for (size_t i = 0; i != functions_.size(); ++i) {
    if (functions_[i].Shape() == shape_index &&
        functions_[i].Type() == FunctionType::Tilt) {
      const unsigned channel_value =
          fixture.Functions()[i]->GetControlValue(snapshot);
      const double max_tilt = Type().MaxTilt();
      const double min_tilt = Type().MinTilt();
      return (max_tilt - min_tilt) * channel_value / ControlValue::MaxUInt() +
             min_tilt;
    }
  }
  return 0.0;
}

double FixtureMode::GetZoom(const Fixture &fixture,
                            const ValueSnapshot &snapshot,
                            size_t shape_index) const {
  for (size_t i = 0; i != functions_.size(); ++i) {
    if (functions_[i].Shape() == shape_index &&
        functions_[i].Type() == FunctionType::Zoom) {
      const unsigned channel_value =
          fixture.Functions()[i]->GetControlValue(snapshot);
      const double max_beam_angle = Type().MaxBeamAngle();
      const double min_beam_angle = Type().MinBeamAngle();
      return (max_beam_angle - min_beam_angle) * channel_value /
                 ControlValue::MaxUInt() +
             min_beam_angle;
    }
  }
  return Type().MinBeamAngle();
}

double FixtureMode::GetPower(const Fixture &fixture,
                             const ValueSnapshot &snapshot) const {
  double power = Type().IdlePower();
  double master_value = 1.0;
  for (size_t i = 0; i != functions_.size(); ++i) {
    const FixtureModeFunction &function = functions_[i];
    if (function.Type() == FunctionType::Master) {
      master_value =
          ControlValue(fixture.Functions()[i]->GetControlValue(snapshot))
              .Ratio();
      power += master_value * function.Power();
    }
  }
  for (size_t i = 0; i != functions_.size(); ++i) {
    const FixtureModeFunction &function = functions_[i];
    if (IsColor(function.Type())) {
      const unsigned channel_value =
          fixture.Functions()[i]->GetControlValue(snapshot);
      power +=
          ControlValue(channel_value).Ratio() * function.Power() * master_value;
    }
  }
  return std::min(power, static_cast<double>(Type().MaxPower()));
}

}  // namespace glight::theatre
