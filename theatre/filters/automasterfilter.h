#ifndef GLIGHT_THEATRE_AUTO_MASTER_FILTER_H_
#define GLIGHT_THEATRE_AUTO_MASTER_FILTER_H_

#include <cassert>

#include "filter.h"

#include "system/optionalnumber.h"

namespace glight::theatre {

class AutoMasterFilter final : public Filter {
 public:
  FilterType GetType() const override { return FilterType::AutoMaster; }

  void Apply(const std::vector<ControlValue>& input,
             std::vector<ControlValue>& output) override {
    unsigned maximum;
    if (master_channel_index_) {
      maximum = 0;
      for (size_t i = 0; i != input.size(); ++i) {
        if (IsColor(InputTypes()[i].Type()) && input[i].UInt() > maximum) {
          maximum = input[i].UInt();
        } else if ((InputTypes()[i].Type() == FunctionType::ColorMacro ||
                    InputTypes()[i].Type() == FunctionType::ColorTemperature) &&
                   input[i]) {
          maximum = ControlValue::MaxUInt();
        }
      }
      maximum =
          std::ceil(std::sqrt(maximum) * std::sqrt(ControlValue::MaxUInt()));
    } else {
      maximum = ControlValue::MaxUInt();
    }
    size_t input_index = 0;
    for (size_t i = 0; i != OutputTypes().size(); ++i) {
      if (OutputTypes()[i].Type() == FunctionType::Master) {
        output[i] = ControlValue(maximum);
      } else if (IsColor(OutputTypes()[i].Type())) {
        output[i] = ControlValue(
            ControlValue::Fraction(input[input_index].UInt(), maximum));
        ++input_index;
      } else {
        output[i] = input[input_index];
        ++input_index;
      }
    }
  }

 protected:
  void DetermineInputTypes() override {
    master_channel_index_.Reset();
    std::vector<FixtureModeFunction> input_functions;
    for (size_t i = 0; i != OutputTypes().size(); ++i) {
      const FunctionType type = OutputTypes()[i].Type();
      if (type == FunctionType::Master) {
        master_channel_index_ = i;
      } else {
        input_functions.emplace_back(OutputTypes()[i]);
      }
    }
    SetInputTypes(std::move(input_functions));
  }

 private:
  system::OptionalNumber<size_t> master_channel_index_;
};

}  // namespace glight::theatre

#endif
