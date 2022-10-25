#include "togglewidget.h"

#include "../eventtransmitter.h"

#include "../dialogs/inputselectdialog.h"

#include "../../theatre/controlvalue.h"
#include "../../theatre/management.h"
#include "../../theatre/presetvalue.h"
#include "../../theatre/sourcevalue.h"

namespace glight::gui {

ToggleWidget::ToggleWidget(theatre::Management &management,
                           EventTransmitter &eventHub, ControlMode mode,
                           char key)
    : ControlWidget(management, eventHub, mode),
      _flashButton(std::string(1, key)),
      _nameLabel("<..>"),
      _holdUpdates(false) {
  _flashButton.set_events(Gdk::BUTTON_PRESS_MASK);
  _flashButton.signal_button_press_event().connect(
      sigc::mem_fun(*this, &ToggleWidget::onFlashButtonPressed), false);
  _flashButton.set_events(Gdk::BUTTON_PRESS_MASK);
  _flashButton.signal_button_release_event().connect(
      sigc::mem_fun(*this, &ToggleWidget::onFlashButtonReleased), false);
  _flashButtonBox.pack_start(_flashButton, false, false, 0);
  _flashButton.show();

  _box.pack_start(_flashButtonBox, false, false, 0);
  _flashButtonBox.set_valign(Gtk::ALIGN_CENTER);
  _flashButtonBox.show();

  _onCheckButton.signal_clicked().connect(
      sigc::mem_fun(*this, &ToggleWidget::onOnButtonClicked));
  _box.pack_start(_onCheckButton, false, false, 3);
  _onCheckButton.show();

  _nameLabel.set_halign(Gtk::ALIGN_START);
  _nameLabel.set_justify(Gtk::JUSTIFY_LEFT);
  _eventBox.add(_nameLabel);
  _nameLabel.show();

  _eventBox.set_events(Gdk::BUTTON_PRESS_MASK);
  _eventBox.signal_button_press_event().connect(
      sigc::mem_fun(*this, &ToggleWidget::onNameLabelClicked));
  _eventBox.show();
  _box.pack_start(_eventBox, true, true, 0);

  add(_box);
  _box.show();
}

void ToggleWidget::onOnButtonClicked() {
  if (!_holdUpdates) {
    unsigned value;
    if (_onCheckButton.get_active())
      value = theatre::ControlValue::MaxUInt();
    else
      value = 0;

    setTargetValue(value);
    SignalValueChange().emit(value);
  }
}

bool ToggleWidget::onFlashButtonPressed(GdkEventButton *) {
  _onCheckButton.set_active(true);
  return false;
}

bool ToggleWidget::onFlashButtonReleased(GdkEventButton *) {
  _onCheckButton.set_active(false);
  return false;
}

bool ToggleWidget::onNameLabelClicked(GdkEventButton *) {
  InputSelectDialog dialog(GetManagement(), GetEventHub());
  if (dialog.run() == Gtk::RESPONSE_OK) {
    Assign(dialog.SelectedInputPreset(), true);
  }
  return true;
}

void ToggleWidget::OnAssigned(bool moveFader) {
  if (GetSourceValue() != nullptr) {
    _nameLabel.set_text(GetSourceValue()->Name());
    if (moveFader) {
      _onCheckButton.set_active(GetSingleSourceValue().Value().UInt() != 0);
    } else {
      if (_onCheckButton.get_active())
        setTargetValue(theatre::ControlValue::MaxUInt());
      else
        setTargetValue(0);
    }
  } else {
    _nameLabel.set_text("<..>");
    if (moveFader) {
      _onCheckButton.set_active(false);
    } else {
      if (_onCheckButton.get_active())
        setTargetValue(theatre::ControlValue::MaxUInt());
      else
        setTargetValue(0);
    }
  }
  if (moveFader) {
    unsigned value;
    if (_onCheckButton.get_active())
      value = theatre::ControlValue::MaxUInt();
    else
      value = 0;
    SignalValueChange().emit(value);
  }
}

void ToggleWidget::MoveSlider() {
  if (GetSourceValue() != nullptr) {
    _onCheckButton.set_active(GetSingleSourceValue().TargetValue() != 0);
    SignalValueChange().emit(GetSingleSourceValue().TargetValue());
  }
}

void ToggleWidget::Toggle() {
  _onCheckButton.set_active(!_onCheckButton.get_active());
}

void ToggleWidget::FullOn() { _onCheckButton.set_active(true); }

void ToggleWidget::FullOff() { _onCheckButton.set_active(false); }

void ToggleWidget::Limit(double value) {
  if (value < theatre::ControlValue::MaxUInt())
    _onCheckButton.set_active(false);
}

}  // namespace glight::gui
