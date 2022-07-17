#include "presetvalue.h"

#include "controllable.h"

namespace glight::theatre {

std::string PresetValue::Title() const {
  return _controllable->InputName(_inputIndex);
}

}  // namespace glight::theatre
