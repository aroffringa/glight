#include "presetvalue.h"

#include "controllable.h"

std::string PresetValue::Title() const {
  return _controllable->InputName(_inputIndex);
}
