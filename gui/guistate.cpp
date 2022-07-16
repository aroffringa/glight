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
    _presetValueDeletedConnection =
        sourceValue->Preset().SignalDelete().connect(
            [&]() { onPresetValueDeleted(); });
}

FaderState::FaderState(const FaderState &source)
    : _sourceValue(source._sourceValue),
      _isToggleButton(source._isToggleButton),
      _newToggleButtonColumn(source._newToggleButtonColumn) {
  if (_sourceValue != nullptr)
    _presetValueDeletedConnection =
        _sourceValue->Preset().SignalDelete().connect(
            [&]() { onPresetValueDeleted(); });
}

FaderState &FaderState::operator=(const FaderState &rhs) {
  _presetValueDeletedConnection.disconnect();
  _sourceValue = rhs._sourceValue;
  if (_sourceValue != nullptr)
    _presetValueDeletedConnection =
        _sourceValue->Preset().SignalDelete().connect(
            [&]() { onPresetValueDeleted(); });
  return *this;
}

void FaderState::SetSourceValue(SourceValue *sourceValue) {
  _presetValueDeletedConnection.disconnect();
  _sourceValue = sourceValue;
  if (sourceValue != nullptr)
    _presetValueDeletedConnection =
        sourceValue->Preset().SignalDelete().connect(
            [&]() { onPresetValueDeleted(); });
}

void FaderState::onPresetValueDeleted() {
  _presetValueDeletedConnection.disconnect();
  _sourceValue = nullptr;
}

void FaderSetupState::ChangeManagement(Management &management) {
  for (FaderState &fader : faders) {
    if (fader.GetSourceValue() != nullptr) {
      Controllable *oldControllable = &fader.GetSourceValue()->Controllable();
      size_t inputIndex = fader.GetSourceValue()->Preset().InputIndex();
      Controllable *newControllable = dynamic_cast<Controllable *>(
          management.GetObjectFromPathIfExists(oldControllable->FullPath()));
      if (newControllable) {
        SourceValue *source =
            management.GetSourceValue(*newControllable, inputIndex);
        fader.SetSourceValue(source);
      } else {
        fader.SetNoSourceValue();
      }
    }
  }
}

}  // namespace glight::gui
