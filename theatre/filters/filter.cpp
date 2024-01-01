#include "filter.h"

#include "automasterfilter.h"
#include "monochromefilter.h"
#include "rgbfilter.h"

namespace glight::theatre {

std::string ToString(FilterType type) {
  switch (type) {
    case FilterType::AutoMaster:
      return "auto-master";
    case FilterType::Monochrome:
      return "monochrome";
    case FilterType::RgbColorspace:
      return "rgb-colorspace";
  }
  return {};
}

FilterType GetFilterType(const std::string& filter_type_string) {
  if (filter_type_string == "auto-master")
    return FilterType::AutoMaster;
  else if (filter_type_string == "monochrome")
    return FilterType::AutoMaster;
  else
    return FilterType::RgbColorspace;
}

std::unique_ptr<Filter> Filter::Make(FilterType type) {
  switch (type) {
    case FilterType::AutoMaster:
      return std::make_unique<AutoMasterFilter>();
    case FilterType::Monochrome:
      return std::make_unique<MonochromeFilter>();
    case FilterType::RgbColorspace:
      return std::make_unique<RgbFilter>();
  }
  return {};
}

}  // namespace glight::theatre
