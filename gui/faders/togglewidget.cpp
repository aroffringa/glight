#include "togglewidget.h"

#include "../eventtransmitter.h"

#include "../dialogs/inputselectdialog.h"

#include "../../theatre/controlvalue.h"
#include "../../theatre/management.h"
#include "../../theatre/presetvalue.h"
#include "../../theatre/sourcevalue.h"

#include "../../system/uniquewithoutordering.h"

namespace glight::gui {

ToggleWidget::ToggleWidget(FaderWindow& fader_window, ControlMode mode,
                           char key)
    : ControlWidget(fader_window, mode),
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

  _iconButton.SignalChanged().connect(
      sigc::mem_fun(*this, &ToggleWidget::onIconClicked));
  _box.pack_start(_iconButton, false, false, 3);
  _iconButton.show();

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

void ToggleWidget::onIconClicked() {
  if (!_holdUpdates) {
    unsigned value;
    if (_iconButton.GetActive())
      value = theatre::ControlValue::MaxUInt();
    else
      value = 0;

    setImmediateValue(value);
    SignalValueChange().emit(value);
  }
}

bool ToggleWidget::onFlashButtonPressed(GdkEventButton* /*unused*/) {
  _iconButton.SetActive(true);
  return false;
}

bool ToggleWidget::onFlashButtonReleased(GdkEventButton* /*unused*/) {
  _iconButton.SetActive(false);
  return false;
}

bool ToggleWidget::onNameLabelClicked(GdkEventButton* /*unused*/) {
  InputSelectDialog dialog(GetManagement(), GetEventHub());
  if (dialog.run() == Gtk::RESPONSE_OK) {
    Assign(dialog.SelectedInputPreset(), true);
  }
  return true;
}

void ToggleWidget::OnAssigned(bool moveFader) {
  if (GetSourceValue() != nullptr) {
    _nameLabel.set_text(GetSourceValue()->Name());
    const theatre::Controllable* controllable =
        &GetSourceValue()->GetControllable();
    const std::vector<theatre::Color> colors =
        controllable->InputColors(GetSourceValue()->InputIndex());
    _iconButton.SetColors(UniqueWithoutOrdering(colors));
    if (moveFader) {
      _iconButton.SetActive(GetSingleSourceValue().Value().UInt() != 0);
    } else {
      if (_iconButton.GetActive())
        setTargetValue(theatre::ControlValue::MaxUInt());
      else
        setTargetValue(0);
    }
  } else {
    _nameLabel.set_text("<..>");
    _iconButton.SetColors({});
    if (moveFader) {
      _iconButton.SetActive(false);
    } else {
      if (_iconButton.GetActive())
        setTargetValue(theatre::ControlValue::MaxUInt());
      else
        setTargetValue(0);
    }
  }
  if (moveFader) {
    unsigned value;
    if (_iconButton.GetActive())
      value = theatre::ControlValue::MaxUInt();
    else
      value = 0;
    SignalValueChange().emit(value);
  }
}

void ToggleWidget::MoveSlider() {
  if (GetSourceValue() != nullptr) {
    const unsigned target_value = GetSingleSourceValue().TargetValue();
    _iconButton.SetActive(target_value != 0);
    SignalValueChange().emit(target_value);
  }
}

void ToggleWidget::Toggle() { _iconButton.SetActive(!_iconButton.GetActive()); }

void ToggleWidget::FullOn() { _iconButton.SetActive(true); }

void ToggleWidget::FullOff() { _iconButton.SetActive(false); }

void ToggleWidget::Limit(double value) {
  if (value < theatre::ControlValue::MaxUInt()) _iconButton.SetActive(false);
}

}  // namespace glight::gui
