#include "controlwidget.h"

#include "../eventtransmitter.h"

#include "../faders/faderwindow.h"

#include "../../theatre/management.h"
#include "../../theatre/presetvalue.h"
#include "../../theatre/sourcevalue.h"

namespace glight::gui {

ControlWidget::ControlWidget(FaderWindow& fader_window, ControlMode mode)
    : _mode(mode),
      fader_window_(fader_window),
      _management(fader_window.GetManagement()),
      _eventHub(fader_window.GetEventTransmitter()),
      _updateConnection(_eventHub.SignalUpdateControllables().connect(
          [&]() { OnTheatreUpdate(); })) {}

ControlWidget::~ControlWidget() { _updateConnection.disconnect(); }

void ControlWidget::Assign(theatre::SourceValue *item, bool moveFader) {
  if (item != _sourceValue) {
    _sourceValue = item;
    OnAssigned(moveFader);
    SignalAssigned().emit();
    if (moveFader) {
      if (item) {
        SignalValueChange().emit(GetSingleSourceValue().Value().UInt());
      }
    } else {
      SignalValueChange().emit(0);
    }
  }
}

void ControlWidget::setTargetValue(unsigned target) {
  if (_sourceValue != nullptr) {
    const unsigned sourceValue = GetSingleSourceValue().Value().UInt();
    const double fadeSpeed =
        (target > sourceValue) ? _fadeUpSpeed : _fadeDownSpeed;
    GetSingleSourceValue().Set(target, fadeSpeed);
  }
}

void ControlWidget::setImmediateValue(unsigned value) {
  if (_sourceValue != nullptr) {
    GetSingleSourceValue().Set(value, 0);
  }
}

double ControlWidget::MAX_SCALE_VALUE() {
  return theatre::ControlValue::MaxUInt() + 1;
}

void ControlWidget::OnTheatreUpdate() {
  if (_sourceValue) {
    // The preset might be removed, if so send reassign
    if (!_management.Contains(*_sourceValue)) {
      _sourceValue = nullptr;
      OnAssigned(true);
    } else {
      // Only if not removed: if preset is renamed, update
      OnAssigned(false);
    }
  }
}

theatre::SingleSourceValue &ControlWidget::GetSingleSourceValue() {
  return _mode == ControlMode::Primary ? _sourceValue->A() : _sourceValue->B();
}

}  // namespace glight::gui
