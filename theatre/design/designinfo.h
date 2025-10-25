#ifndef GLIGHT_THEATRE_DESIGN_INFO_H_
#define GLIGHT_THEATRE_DESIGN_INFO_H_

#include <string>
#include <string_view>
#include <vector>

#include "system/trackableptr.h"

#include "theatre/colordeduction.h"

namespace glight::theatre {

class Controllable;
class Folder;
class Management;

struct DesignInfo {
  Management* management;
  Folder* destination;
  std::string name;
  const std::vector<system::ObservingPtr<Controllable>>* controllables;
  ColorDeduction deduction;
};

std::string GetValidName(const DesignInfo& design,
                         std::string_view alternative_name);

}  // namespace glight::theatre

#endif
