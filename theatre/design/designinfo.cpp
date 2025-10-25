#include "designinfo.h"

#include "theatre/folder.h"

namespace glight::theatre {

std::string GetValidName(const DesignInfo& design,
                         std::string_view alternative_name) {
  if (design.name.empty())
    return design.destination->GetAvailableName(alternative_name);
  else if (design.destination->GetChildIfExists(design.name))
    return design.destination->GetAvailableName(design.name);
  else
    return design.name;
}

}  // namespace glight::theatre
