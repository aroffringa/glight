#ifndef GLIGHT_THEATRE_COLOR_TEMPERATURE_FILTER_H_
#define GLIGHT_THEATRE_COLOR_TEMPERATURE_FILTER_H_

#include <cassert>

#include "filter.h"

#include "system/colortemperature.h"

namespace glight::theatre {

class ColorTemperatureFilter final : public Filter {
 public:
  FilterType GetType() const override { return FilterType::ColorTemperature; }

  void Apply(const std::vector<ControlValue>& input,
             std::vector<ControlValue>& output) override {
    if (enabled_) {
      const ControlValue red = input[0];
      const ControlValue green = input[1];
      const ControlValue blue = input[2];
      const double temperature =
          system::RgbToTemperature(red.Ratio(), green.Ratio(), blue.Ratio());
      constexpr double min_temperature = 2800;
      constexpr double max_temperature = 8000;
      output[master_channel_index_] = Max(red, green, blue);
      ;
      output[color_temperature_channel_index_] =
          ControlValue::FromRatio((temperature - min_temperature) /
                                  (max_temperature - min_temperature));
    }
    size_t input_index = 3;
    for (size_t i = 0; i != OutputTypes().size(); ++i) {
      if (OutputTypes()[i].Type() != FunctionType::Master &&
          OutputTypes()[i].Type() != FunctionType::ColorTemperature) {
        output[i] = input[input_index];
        ++input_index;
      }
    }
  }

 protected:
  void DetermineInputTypes() override {
    system::OptionalNumber<size_t> master;
    system::OptionalNumber<size_t> temperature;
    std::vector<FixtureModeFunction> input_types{
        FixtureModeFunction(FunctionType::Red, 0, {}, 0),
        FixtureModeFunction(FunctionType::Green, 0, {}, 0),
        FixtureModeFunction(FunctionType::Blue, 0, {}, 0)};
    for (size_t i = 0; i != OutputTypes().size(); ++i) {
      const FunctionType type = OutputTypes()[i].Type();
      if (type == FunctionType::Master)
        master = i;
      else if (type == FunctionType::ColorTemperature)
        temperature = i;
      else
        input_types.emplace_back(OutputTypes()[i]);
    }
    enabled_ = master && temperature;
    if (enabled_) {
      master_channel_index_ = *master;
      color_temperature_channel_index_ = *temperature;
    }
    SetInputTypes(std::move(input_types));
  }

 private:
  /**
   * If the fixture has not both a master channel and a color temperature,
   * enabled_ is set to false and the filter will not do anything.
   */
  bool enabled_ = false;
  size_t master_channel_index_ = 0;
  size_t color_temperature_channel_index_ = 0;
};

}  // namespace glight::theatre

#endif
