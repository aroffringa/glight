#include "controlwidget.h"

#include "../eventtransmitter.h"

#include "../faders/faderwindow.h"

#include "../../theatre/management.h"
#include "../../theatre/presetvalue.h"
#include "../../theatre/sourcevalue.h"

#include "../dialogs/inputselectdialog.h"

namespace glight::gui {

using theatre::SourceValue;

ControlWidget::ControlWidget(FaderWindow& fader_window, FaderState& state,
                             ControlMode mode)
    : _mode(mode),
      _state(state),
      fader_window_(fader_window),
      _management(fader_window.GetManagement()),
      _eventHub(fader_window.GetEventTransmitter()),
      _updateConnection(_eventHub.SignalUpdateControllables().connect(
          [&]() { OnTheatreUpdate(); })) {}

ControlWidget::~ControlWidget() { _updateConnection.disconnect(); }

bool ControlWidget::IsAssigned() const {
  for (const theatre::SourceValue* source : sources_) {
    if (source) return true;
  }
  return false;
}

void ControlWidget::Assign(const std::vector<theatre::SourceValue*>& sources,
                           bool sync_fader) {
  if (sources != sources_) {
    sources_ = sources;
    OnAssigned(sync_fader);
    SignalAssigned().emit();
    if (sync_fader) {
      if (IsAssigned()) {
        // TODO sync fader
        SignalValueChange().emit();
      }
    } else {
      SignalValueChange().emit();
    }
  }
}

void ControlWidget::setTargetValue(size_t source_index, unsigned target) {
  if (source_index < sources_.size() && sources_[source_index]) {
    const unsigned source = GetSingleSourceValue(source_index).Value().UInt();
    const double fadeSpeed = (target > source) ? _fadeUpSpeed : _fadeDownSpeed;
    GetSingleSourceValue(source_index).Set(target, fadeSpeed);
  }
}

void ControlWidget::setImmediateValue(size_t source_index, unsigned value) {
  if (source_index < sources_.size() && sources_[source_index]) {
    GetSingleSourceValue(source_index).Set(value, 0);
  }
}

double ControlWidget::MAX_SCALE_VALUE() {
  return theatre::ControlValue::MaxUInt() + 1;
}

void ControlWidget::OnTheatreUpdate() {
  if (IsAssigned()) {
    // The preset might be removed, if so send reassign
    bool all_sources_exist = true;
    for (const SourceValue* source : sources_) {
      if (source && !_management.Contains(*source)) {
        all_sources_exist = false;
        break;
      }
    }
    if (all_sources_exist) {
      // If not removed: update because controllables might be renamed
      OnAssigned(false);
    } else {
      Unassign();
    }
  }
}

theatre::SingleSourceValue& ControlWidget::GetSingleSourceValue(size_t index) {
  return _mode == ControlMode::Primary ? sources_[index]->A()
                                       : sources_[index]->B();
}

void ControlWidget::ShowAssignDialog() {
  InputSelectDialog dialog(GetManagement(), GetEventHub(), false);
  if (dialog.run() == Gtk::RESPONSE_OK) {
    Assign({dialog.SelectedSourceValue()}, true);
  }
}

}  // namespace glight::gui
