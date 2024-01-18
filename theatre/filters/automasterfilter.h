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
        if (IsColor(InputTypes()[i]) && input[i].UInt() > maximum) {
          maximum = input[i].UInt();
        }
      }
      maximum =
          std::ceil(std::sqrt(maximum) * std::sqrt(ControlValue::MaxUInt()));
    } else {
      maximum = ControlValue::MaxUInt();
    }
    size_t input_index = 0;
    for (size_t i = 0; i != OutputTypes().size(); ++i) {
      if (OutputTypes()[i] == FunctionType::Master) {
        output[i] = ControlValue(maximum);
      } else if (IsColor(OutputTypes()[i])) {
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
    std::vector<FunctionType> input_types;
    for (size_t i = 0; i != OutputTypes().size(); ++i) {
      const FunctionType type = OutputTypes()[i];
      if (type == FunctionType::Master) {
        master_channel_index_ = i;
      } else {
        input_types.emplace_back(type);
      }
    }
    SetInputTypes(std::move(input_types));
  }

 private:
  system::OptionalNumber<size_t> master_channel_index_;
};

}  // namespace glight::theatre

#endif
