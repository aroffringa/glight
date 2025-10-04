#ifndef GLIGHT_THEATRE_MONOCHROME_FILTER_H_
#define GLIGHT_THEATRE_MONOCHROME_FILTER_H_

#include <cassert>

#include "filter.h"

#include "../functiontype.h"

namespace glight::theatre {

class MonochromeFilter final : public Filter {
 public:
  FilterType GetType() const override { return FilterType::Monochrome; }

  void Apply(const std::vector<ControlValue>& input,
             std::vector<ControlValue>& output) override {
    assert(input.size() == InputTypes().size());
    assert(output.size() == OutputTypes().size());
    size_t input_index = 1;
    for (size_t output_index = 0; output_index != OutputTypes().size();
         ++output_index) {
      if (IsColor(OutputTypes()[output_index].Type())) {
        output[output_index] = input[0];
      } else {
        output[output_index] = input[input_index];
        ++input_index;
      }
    }
  }

 protected:
  void DetermineInputTypes() override {
    std::vector<FixtureModeFunction> input_types{
        FixtureModeFunction(FunctionType::White, 0, {}, 0)};
    for (const FixtureModeFunction& function : OutputTypes()) {
      if (!IsColor(function.Type())) input_types.emplace_back(function);
    }
    SetInputTypes(std::move(input_types));
  }
};

}  // namespace glight::theatre

#endif
