#include "combocontrolwidget.h"

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
  box_.pack_start(description_label_, true, true, 0);
  description_label_.show();

  auto mouse_handler = [&](GdkEventButton *button) {
    return HandleRightRelease(button);
  };
  model_ = Gtk::ListStore::create(columns_);
  combo_.signal_changed().connect([&]() { OnChanged(); });
  combo_.signal_button_release_event().connect(mouse_handler);
  combo_.set_model(model_);
  combo_.pack_start(columns_.title_);
  box_.pack_start(combo_);
  combo_.show();

  event_box_.add(box_);
  box_.show();

  event_box_.set_events(Gdk::BUTTON_PRESS_MASK);
  event_box_.signal_button_press_event().connect([&](GdkEventButton *button) {
    if (button->button == 1) {
      ShowAssignDialog();
      return true;
    }
    return false;
  });
  event_box_.signal_button_release_event().connect(mouse_handler);
  add(event_box_);
  event_box_.show();

  UpdateDisplaySettings();
  update_display_settings_connection_ =
      State().SignalChange().connect([&]() { UpdateDisplaySettings(); });
}

ComboControlWidget::~ComboControlWidget() {
  update_display_settings_connection_.disconnect();
}

SourceValue *ComboControlWidget::SelectedSource() const {
  Gtk::TreeModel::iterator selected = combo_.get_active();
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
        return child;
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

bool ComboControlWidget::HandleRightRelease(GdkEventButton *event) {
  if (event->button == 3) {
    std::unique_ptr<ControlMenu> &menu = PrepareMenu();
    menu->AddExtraItem("Set description...",
                       [&]() { OpenDescriptionDialog(); });
    menu->popup_at_pointer(reinterpret_cast<const GdkEvent *>(event));
    return true;
  } else {
    return false;
  }
}

void ComboControlWidget::UpdateDisplaySettings() {
  description_label_.set_visible(State().DisplayName());
  if (State().Label().empty())
    description_label_.set_text("<No description>");
  else
    description_label_.set_text(State().Label());
}

void ComboControlWidget::OpenDescriptionDialog() {
  StringInputDialog dialog("Combo control description",
                           "New description:", description_label_.get_text());
  if (dialog.run() == Gtk::RESPONSE_OK) {
    State().SetLabel(dialog.Value());
  }
}

}  // namespace glight::gui
