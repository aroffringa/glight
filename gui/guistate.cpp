#include "guistate.h"

#include "../theatre/controllable.h"
#include "../theatre/management.h"
#include "../theatre/sourcevalue.h"

namespace glight::gui {

using theatre::Controllable;
using theatre::Management;
using theatre::SourceValue;

FaderState::FaderState(SourceValue *sourceValue)
    : _sourceValue(sourceValue),
      _isToggleButton(false),
      _newToggleButtonColumn(false) {
  if (sourceValue != nullptr)
    _sourceValueDeletedConnection =
        sourceValue->SignalDelete().connect([&]() { onPresetValueDeleted(); });
}

FaderState::FaderState(const FaderState &source)
    : _sourceValue(source._sourceValue),
      _isToggleButton(source._isToggleButton),
      _newToggleButtonColumn(source._newToggleButtonColumn) {
  if (_sourceValue != nullptr)
    _sourceValueDeletedConnection =
        _sourceValue->SignalDelete().connect([&]() { onPresetValueDeleted(); });
}

FaderState &FaderState::operator=(const FaderState &rhs) {
  _sourceValueDeletedConnection.disconnect();
  _sourceValue = rhs._sourceValue;
  if (_sourceValue != nullptr)
    _sourceValueDeletedConnection =
        _sourceValue->SignalDelete().connect([&]() { onPresetValueDeleted(); });
  return *this;
}

void FaderState::SetSourceValue(SourceValue *sourceValue) {
  _sourceValueDeletedConnection.disconnect();
  _sourceValue = sourceValue;
  if (sourceValue != nullptr)
    _sourceValueDeletedConnection =
        sourceValue->SignalDelete().connect([&]() { onPresetValueDeleted(); });
}

void FaderState::onPresetValueDeleted() {
  _sourceValueDeletedConnection.disconnect();
  _sourceValue = nullptr;
}

}  // namespace glight::gui
