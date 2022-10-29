#include "faderwidget.h"

#include "../dialogs/inputselectdialog.h"

#include "../../theatre/controllable.h"
#include "../../theatre/fixturecontrol.h"
#include "../../theatre/presetvalue.h"
#include "../../theatre/sourcevalue.h"

namespace glight::gui {

using theatre::ControlValue;

FaderWidget::FaderWidget(theatre::Management &management,
                         EventTransmitter &eventHub, ControlMode mode, char key)
    : ControlWidget(management, eventHub, mode),
      _scale(0, ControlValue::MaxUInt() + ControlValue::MaxUInt() / 100,
             (ControlValue::MaxUInt() + 1) / 100),
      _flashButton(std::string(1, key)),
      _nameLabel("<..>"),
      _holdUpdates(false) {
  if (GetMode() == ControlMode::Primary) {
    _fadeUpButton.set_image_from_icon_name("go-up");
    _fadeUpButton.signal_clicked().connect([&]() { onFadeUp(); });
    _box.pack_start(_fadeUpButton, false, false, 0);
    _fadeUpButton.show();
  }

  _scale.set_inverted(true);
  _scale.set_draw_value(false);
  _scale.set_vexpand(true);
  _scale.signal_value_changed().connect(
      sigc::mem_fun(*this, &FaderWidget::onScaleChange));
  _box.pack_start(_scale, true, true, 0);
  _scale.show();

  if (GetMode() == ControlMode::Primary) {
    _fadeDownButton.set_image_from_icon_name("go-down");
    _fadeDownButton.signal_clicked().connect([&]() { onFadeDown(); });
    _box.pack_start(_fadeDownButton, false, false, 0);
    _fadeDownButton.show();

    _flashButton.set_events(Gdk::BUTTON_PRESS_MASK);
    _flashButton.signal_button_press_event().connect(
        sigc::mem_fun(*this, &FaderWidget::onFlashButtonPressed), false);
    _flashButton.set_events(Gdk::BUTTON_PRESS_MASK);
    _flashButton.signal_button_release_event().connect(
        sigc::mem_fun(*this, &FaderWidget::onFlashButtonReleased), false);
    _box.pack_start(_flashButton, false, false, 0);
    _flashButton.show();
  }

  _onCheckButton.set_halign(Gtk::ALIGN_CENTER);
  _onCheckButton.SignalClicked().connect(
      sigc::mem_fun(*this, &FaderWidget::onOnButtonClicked));
  _box.pack_start(_onCheckButton, false, false, 0);
  _onCheckButton.show();

  // pack_start(_eventBox, false, false, 0);
  _eventBox.set_events(Gdk::BUTTON_PRESS_MASK);
  _eventBox.show();

  _eventBox.signal_button_press_event().connect(
      sigc::mem_fun(*this, &FaderWidget::onNameLabelClicked));
  _eventBox.add(_nameLabel);
  _nameLabel.show();

  add(_box);
  _box.show();
}

void FaderWidget::onOnButtonClicked() {
  if (!_holdUpdates) {
    _holdUpdates = true;
    if (_onCheckButton.GetActive())
      _scale.set_value(ControlValue::MaxUInt());
    else
      _scale.set_value(0);
    _holdUpdates = false;

    setImmediateValue(_scale.get_value());
    SignalValueChange().emit(_scale.get_value());
  }
}

bool FaderWidget::onFlashButtonPressed(GdkEventButton *) {
  _scale.set_value(ControlValue::MaxUInt());
  return false;
}

bool FaderWidget::onFlashButtonReleased(GdkEventButton *) {
  _scale.set_value(0);
  return false;
}

void FaderWidget::onScaleChange() {
  if (!_holdUpdates) {
    _holdUpdates = true;
    _onCheckButton.SetActive(_scale.get_value() != 0);
    _holdUpdates = false;

    setImmediateValue(_scale.get_value());
    SignalValueChange().emit(_scale.get_value());
  }
}

bool FaderWidget::onNameLabelClicked(GdkEventButton *) {
  InputSelectDialog dialog(GetManagement(), GetEventHub());
  if (dialog.run() == Gtk::RESPONSE_OK) {
    Assign(dialog.SelectedInputPreset(), true);
  }
  return true;
}

void FaderWidget::OnAssigned(bool moveFader) {
  const theatre::SourceValue* source = GetSourceValue();
  if (source) {
    _nameLabel.set_text(GetSourceValue()->Name());
    if (moveFader) {
      _scale.set_value(GetSingleSourceValue().Value().UInt());
    } else {
      setImmediateValue(_scale.get_value());
    }
    
    const theatre::Controllable* controllable = &source->GetControllable();
    _onCheckButton.SetColors(controllable->InputColors(source->InputIndex()));
  } else {
    _nameLabel.set_text("<..>");
    _onCheckButton.SetColors({});
    if (moveFader) {
      _scale.set_value(0);
    } else {
      setImmediateValue(_scale.get_value());
    }
  }
}

void FaderWidget::MoveSlider() {
  if (GetSourceValue() != nullptr) {
    const bool hold = _holdUpdates;
    _holdUpdates = true;
    const unsigned value = GetSingleSourceValue().Value().UInt();
    _scale.set_value(value);
    _onCheckButton.SetActive(value != 0);
    _holdUpdates = hold;
    SignalValueChange().emit(_scale.get_value());
  }
}

void FaderWidget::Toggle() {
  _onCheckButton.SetActive(!_onCheckButton.GetActive());
}

void FaderWidget::FullOn() { _scale.set_value(ControlValue::MaxUInt()); }

void FaderWidget::FullOff() { _scale.set_value(0); }

void FaderWidget::onFadeUp() { setTargetValue(ControlValue::MaxUInt()); }

void FaderWidget::onFadeDown() { setTargetValue(0); }

}  // namespace glight::gui
