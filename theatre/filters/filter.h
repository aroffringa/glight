#ifndef THEATRE_FILTER_H_
#define THEATRE_FILTER_H_

#include <string>

#include "../controlvalue.h"
#include "../folderobject.h"
#include "../fixturemodefunction.h"

namespace glight::theatre {

enum class FilterType {
  AutoMaster,
  ColorTemperature,
  Monochrome,
  RgbColorspace
};

std::string ToString(FilterType type);
FilterType GetFilterType(const std::string& filter_type_string);

class Filter {
 public:
  Filter() noexcept = default;

  virtual ~Filter() = default;

  static std::unique_ptr<Filter> Make(FilterType type);

  const std::vector<FixtureModeFunction>& InputTypes() const {
    return input_types_;
  }
  const std::vector<FixtureModeFunction>& OutputTypes() const {
    return output_types_;
  }

  virtual FilterType GetType() const = 0;

  /**
   * Determines the input types that this filter provides from the output
   * types, and calls @ref SetInputTypes() accordingly.
   * This function is called when the filter is connected to a filter
   * or a fixture.
   *
   * For example, a filter that converts an RGB fixture to white only
   * might replace the R, G and B functions by a single white function,
   * and copy other functions (master channel, strobe, etc.) from the output.
   */
  virtual void DetermineInputTypes() = 0;

  /**
   * Set the output types, which connects this filter to another filter or
   * fixture and adapts the input types accordingly.
   */
  void SetOutputTypes(std::vector<FixtureModeFunction> output_functions) {
    output_types_ = std::move(output_functions);
    DetermineInputTypes();
  }

  /**
   * Applies the filter to the input and sets the output accordingly.
   * @param input should have a size of InputTypes().size().
   * @param output Output parameter. On input, should have a size equal to
   * OutputTypes().size().
   */
  virtual void Apply(const std::vector<ControlValue>& input,
                     std::vector<ControlValue>& output) = 0;

 protected:
  void SetInputTypes(std::vector<FixtureModeFunction> input_types) {
    input_types_ = std::move(input_types);
  }

 private:
  std::vector<FixtureModeFunction> input_types_;
  std::vector<FixtureModeFunction> output_types_;
};

}  // namespace glight::theatre

#endif
