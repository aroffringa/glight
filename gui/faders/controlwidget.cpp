#include "controlwidget.h"

#include "gui/eventtransmitter.h"
#include "gui/instance.h"

#include "gui/dialogs/inputselectdialog.h"

#include "gui/faders/controlmenu.h"
#include "gui/faders/faderwindow.h"

#include "gui/state/faderstate.h"

#include "theatre/management.h"
#include "theatre/presetvalue.h"
#include "theatre/sourcevalue.h"

namespace glight::gui {

using theatre::SourceValue;

ControlWidget::ControlWidget(FaderWindow& fader_window, FaderState& state,
                             ControlMode mode)
    : _mode(mode),
      _state(state),
      fader_window_(fader_window),
      _updateConnection(Instance::Events().SignalUpdateControllables().connect(
          [&]() { OnTheatreUpdate(); })),
      state_change_connection_(
          state.SignalChange().connect([&]() { OnStateChange(); })) {}

ControlWidget::~ControlWidget() {
  _updateConnection.disconnect();
  state_change_connection_.disconnect();
}

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
      if (source && !Instance::Management().Contains(*source)) {
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

theatre::SingleSourceValue& ControlWidget::GetSingleSourceValue(
    size_t index) const {
  return sources_[index]->AorB(_mode == ControlMode::Primary);
}

theatre::SingleSourceValue& ControlWidget::GetSingleSourceValue(
    theatre::SourceValue& source) const {
  return source.AorB(_mode == ControlMode::Primary);
}

void ControlWidget::ShowAssignDialog() {
  InputSelectDialog dialog(false);
  if (dialog.run() == Gtk::RESPONSE_OK) {
    const bool allow_multi = DefaultSourceCount() == 1;
    if (allow_multi) {
      std::vector<SourceValue*> sources = GetSourceValues();
      sources.emplace_back(dialog.SelectedSourceValue());
      Assign(sources, true);
    } else {
      Assign({dialog.SelectedSourceValue()}, true);
    }
  }
}

void ControlWidget::OnStateChange() {
  if (sources_ != _state.GetSourceValues()) {
    Assign(_state.GetSourceValues(), true);
  }
}

std::unique_ptr<ControlMenu>& ControlWidget::PrepareMenu() {
  std::unique_ptr<ControlMenu>& menu = GetFaderWindow().GetControlMenu();
  menu = std::make_unique<ControlMenu>(State());
  menu->SignalAssign().connect([&]() { ShowAssignDialog(); });
  menu->SignalUnassign().connect([&]() { Assign({}, true); });
  menu->SignalToggleName().connect(
      [&]() { State().SetDisplayName(menu->DisplayName()); });
  return menu;
}

}  // namespace glight::gui
