#include "combocontrolwidget.h"

#include <gtkmm/gestureclick.h>

#include "controlmenu.h"
#include "faderwindow.h"

#include "gui/state/faderstate.h"

#include "gui/dialogs/inputselectdialog.h"
#include "gui/dialogs/stringinputdialog.h"

#include "gui/eventtransmitter.h"
#include "gui/instance.h"

#include "theatre/controlvalue.h"
#include "theatre/management.h"
#include "theatre/sourcevalue.h"

namespace glight::gui {

using theatre::ControlValue;
using theatre::SourceValue;

ComboControlWidget::ComboControlWidget(FaderWindow &fader_window,
                                       FaderState &state, ControlMode mode,
                                       char key)
    : ControlWidget(fader_window, state, mode) {
  set_orientation(Gtk::Orientation::VERTICAL);
  append(description_label_);
  description_label_.show();

  auto mouse_handler = [&](int, double, double) {
    return HandleRightRelease();
  };
  model_ = Gtk::ListStore::create(columns_);
  combo_.signal_changed().connect([&]() { OnChanged(); });
  auto combo_gesture = Gtk::GestureClick::create();
  combo_gesture->set_button(3);
  combo_gesture->signal_released().connect(mouse_handler);
  combo_.add_controller(combo_gesture);
  combo_.set_model(model_);
  combo_.set_entry_text_column(columns_.title_);
  append(combo_);
  combo_.show();

  auto gesture = Gtk::GestureClick::create();
  gesture->set_button(1);
  gesture->signal_pressed().connect(
      [&](int, double, double) { ShowAssignDialog(); });
  add_controller(gesture);

  SetDefaultSourceCount(0);
  UpdateDisplaySettings();
  update_display_settings_connection_ =
      State().SignalChange().connect([&]() { UpdateDisplaySettings(); });
}

ComboControlWidget::~ComboControlWidget() {
  update_display_settings_connection_.disconnect();
}

SourceValue *ComboControlWidget::SelectedSource() const {
  Gtk::TreeModel::const_iterator selected = combo_.get_active();
  if (selected)
    return (*selected)[columns_.source_];
  else
    return nullptr;
}

void ComboControlWidget::OnAssigned(bool moveFader) {
  const SourceValue *active_source = SelectedSource();
  const std::vector<SourceValue *> &source_values = GetSourceValues();
  model_->clear();
  for (SourceValue *source : source_values) {
    const theatre::Controllable *controllable = &source->GetControllable();
    Gtk::ListStore::iterator iter = model_->append();
    Gtk::ListStore::Row row = *iter;
    row[columns_.title_] = controllable->InputName(source->InputIndex());
    row[columns_.source_] = source;
    if (source == active_source && !moveFader) combo_.set_active(iter);
    if (moveFader) {
      const theatre::SingleSourceValue &value = GetSingleSourceValue(*source);
      if (value.Value()) combo_.set_active(iter);
    }
  }
  if (moveFader) {
    SignalValueChange().emit();
  }
  if (!SelectedSource() && !source_values.empty()) {
    combo_.set_active(0);
  }
}

void ComboControlWidget::OnChanged() {
  theatre::SourceValue *selected = SelectedSource();
  if (selected) {
    GetSingleSourceValue(*selected).Set(ControlValue::MaxUInt());
  }
  for (Gtk::TreeRow child : model_->children()) {
    SourceValue *source = child[columns_.source_];
    if (source && source != selected) {
      GetSingleSourceValue(*source).Set(0);
    }
  }
}

Gtk::ListStore::iterator ComboControlWidget::FirstNonZeroValue() const {
  for (Gtk::TreeRow child : model_->children()) {
    SourceValue *source = child[columns_.source_];
    if (source) {
      if (GetSingleSourceValue(*source).Value()) {
        return child.get_iter();
      }
    }
  }
  return Gtk::ListStore::iterator();
}

void ComboControlWidget::SyncFader() {
  if (IsAssigned()) {
    const SourceValue *selected_source = SelectedSource();
    Gtk::ListStore::iterator first_non_zero = FirstNonZeroValue();
    if (first_non_zero) {
      const SourceValue *active_source = (*first_non_zero)[columns_.source_];
      if (selected_source != active_source) {
        combo_.set_active(first_non_zero);
        SignalValueChange().emit();
      }
    }
  }
}

void ComboControlWidget::Toggle() {
  Gtk::ListStore::iterator active = FirstNonZeroValue();
  if (active) {
    ++active;
    if (!active) {
      active = model_->children().begin();
    }
    if (active) {
      combo_.set_active(active);
      SignalValueChange().emit();
    }
  }
}

void ComboControlWidget::FlashOn() { Toggle(); }

void ComboControlWidget::FlashOff() { Toggle(); }

void ComboControlWidget::Limit(double value) {}

void ComboControlWidget::HandleRightRelease() {
  std::unique_ptr<ControlMenu> &menu = PrepareMenu();
  menu->AddExtraItem("Set description...", [&]() { OpenDescriptionDialog(); });
  menu->popup();
}

void ComboControlWidget::UpdateDisplaySettings() {
  description_label_.set_visible(State().DisplayName());
  if (State().Label().empty())
    description_label_.set_text("<No description>");
  else
    description_label_.set_text(State().Label());
}

void ComboControlWidget::OpenDescriptionDialog() {
  dialog_ = std::make_unique<StringInputDialog>(
      "Combo control description",
      "New description:", description_label_.get_text());
  dialog_->signal_response().connect([this](int response) {
    if (response == Gtk::ResponseType::OK) {
      StringInputDialog &dialog = static_cast<StringInputDialog &>(*dialog_);
      State().SetLabel(dialog.Value());
    }
  });
  dialog_->show();
}

}  // namespace glight::gui
