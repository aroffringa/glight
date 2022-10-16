#include "controlwidget.h"

#include "../eventtransmitter.h"

#include "../../theatre/management.h"
#include "../../theatre/presetvalue.h"
#include "../../theatre/sourcevalue.h"

namespace glight::gui {

ControlWidget::ControlWidget(theatre::Management &management,
                             EventTransmitter &eventHub)
    : _management(&management),
      _eventHub(eventHub),
      _updateConnection(_eventHub.SignalUpdateControllables().connect(
          [&]() { OnTheatreUpdate(); })) {}

ControlWidget::~ControlWidget() { _updateConnection.disconnect(); }

void ControlWidget::Assign(theatre::SourceValue *item, bool moveFader) {
  if (item != _sourceValue) {
    _sourceValue = item;
    OnAssigned(moveFader);
    SignalAssigned().emit();
    if (moveFader) SignalValueChange().emit(_sourceValue->A().Value().UInt());
  }
}

void ControlWidget::setValue(unsigned target) {
  if (GetSourceValue() != nullptr) {
    const unsigned sourceValue = GetSourceValue()->A().Value();
    const double fadeSpeed =
        (target > sourceValue) ? _fadeUpSpeed : _fadeDownSpeed;
    GetSourceValue()->A().Set(target, fadeSpeed);
  }
}

double ControlWidget::MAX_SCALE_VALUE() {
  return theatre::ControlValue::MaxUInt() + 1;
}

void ControlWidget::OnTheatreUpdate() {
  if (_sourceValue) {
    // The preset might be removed, if so send reassign
    if (!_management->Contains(*_sourceValue)) {
      _sourceValue = nullptr;
      OnAssigned(true);
    } else {
      // Only if not removed: if preset is renamed, update
      OnAssigned(false);
    }
  }
}

void ControlWidget::ChangeManagement(theatre::Management &management,
                                     bool moveSliders) {
  if (_sourceValue == nullptr) {
    _management = &management;
  } else {
    std::string controllablePath = _sourceValue->GetControllable().FullPath();
    size_t input = _sourceValue->InputIndex();
    _management = &management;
    theatre::Controllable *controllable = dynamic_cast<theatre::Controllable *>(
        _management->GetObjectFromPathIfExists(controllablePath));
    theatre::SourceValue *sv;
    if (controllable)
      sv = _management->GetSourceValue(*controllable, input);
    else
      sv = nullptr;
    if (sv == nullptr)
      Unassign();
    else {
      Assign(sv, moveSliders);
    }
  }
}

}  // namespace glight::gui
