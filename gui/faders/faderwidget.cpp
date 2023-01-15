#include "faderwidget.h"

#include "../dialogs/inputselectdialog.h"

#include "../../theatre/controllable.h"
#include "../../theatre/fixturecontrol.h"
#include "../../theatre/presetvalue.h"
#include "../../theatre/sourcevalue.h"

#include "../../system/uniquewithoutordering.h"

namespace glight::gui {

using theatre::ControlValue;

FaderWidget::FaderWidget(theatre::Management &management,
                         EventTransmitter &eventHub, ControlMode mode, char key)
    : ControlWidget(management, eventHub, mode),
      _scale(0, ControlValue::MaxUInt() + ControlValue::MaxUInt() / 100,
             (ControlValue::MaxUInt() + 1) / 100),
      _flashButton(std::string(1, key)),
      _nameLabel("<..>") {
  if (GetMode() == ControlMode::Primary) {
    _fadeUpButton.set_image_from_icon_name("go-up");
    _fadeUpButton.signal_clicked().connect([&]() { onFadeUp(); });
    _fadeUpButton.set_halign(Gtk::ALIGN_CENTER);
    _fadeUpButton.set_valign(Gtk::ALIGN_START);
    _fadeUpButton.set_margin_top(10);
    _overlay.add_overlay(_fadeUpButton);
  }

  _scale.set_inverted(true);
  _scale.set_draw_value(false);
  _scale.set_vexpand(true);
  _scale.signal_value_changed().connect(
      sigc::mem_fun(*this, &FaderWidget::onScaleChange));
  _overlay.add(_scale);
  _scale.show();

  _mouseInBox.set_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
  _mouseInBox.signal_enter_notify_event().connect(
      [&](GdkEventCrossing *) {
        ShowFadeButtons(true);
        return false;
      },
      false);
  _mouseInBox.signal_leave_notify_event().connect(
      [&](GdkEventCrossing *) {
        ShowFadeButtons(false);
        return false;
      },
      false);

  _box.pack_start(_mouseInBox, true, true, 0);
  _mouseInBox.show();

  _mouseInBox.add(_overlay);
  _overlay.show();

  if (GetMode() == ControlMode::Primary) {
    _fadeDownButton.set_image_from_icon_name("go-down");
    _fadeDownButton.signal_clicked().connect([&]() { onFadeDown(); });
    _fadeDownButton.set_halign(Gtk::ALIGN_CENTER);
    _fadeDownButton.set_valign(Gtk::ALIGN_END);
    _fadeDownButton.set_margin_bottom(10);
    _overlay.add_overlay(_fadeDownButton);

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
  _onCheckButton.SignalChanged().connect(
      sigc::mem_fun(*this, &FaderWidget::onOnButtonClicked));
  _box.pack_start(_onCheckButton, false, false, 0);
  _onCheckButton.show();

  _labelEventBox.set_events(Gdk::BUTTON_PRESS_MASK);
  _labelEventBox.show();

  _labelEventBox.signal_button_press_event().connect(
      sigc::mem_fun(*this, &FaderWidget::onNameLabelClicked));
  _labelEventBox.add(_nameLabel);
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

bool FaderWidget::onFlashButtonPressed(GdkEventButton * /*unused*/) {
  _scale.set_value(ControlValue::MaxUInt());
  return false;
}

bool FaderWidget::onFlashButtonReleased(GdkEventButton * /*unused*/) {
  _scale.set_value(0);
  return false;
}

void FaderWidget::onScaleChange() {
  if (!_holdUpdates) {
    _holdUpdates = true;
    const double value = _scale.get_value();
    _onCheckButton.SetActive(value != 0);
    if (_mouseIn) {
      _fadeUpButton.set_visible(value < ControlValue::MaxUInt() * 3 / 4);
      _fadeDownButton.set_visible(value >= ControlValue::MaxUInt() * 1 / 4);
    }
    _holdUpdates = false;

    setImmediateValue(_scale.get_value());
    SignalValueChange().emit(_scale.get_value());
  }
}

bool FaderWidget::onNameLabelClicked(GdkEventButton * /*unused*/) {
  InputSelectDialog dialog(GetManagement(), GetEventHub());
  if (dialog.run() == Gtk::RESPONSE_OK) {
    Assign(dialog.SelectedInputPreset(), true);
  }
  return true;
}

void FaderWidget::OnAssigned(bool moveFader) {
  const theatre::SourceValue *source = GetSourceValue();
  if (source) {
    _nameLabel.set_text(GetSourceValue()->Name());
    if (moveFader) {
      _scale.set_value(GetSingleSourceValue().Value().UInt());
    } else {
      setImmediateValue(_scale.get_value());
    }

    const theatre::Controllable *controllable = &source->GetControllable();
    const std::vector<theatre::Color> colors =
        controllable->InputColors(source->InputIndex());
    _onCheckButton.SetColors(UniqueWithoutOrdering(colors));
  } else {
    _nameLabel.set_text("<..>");
    _onCheckButton.SetColors({});
    _fadeUpButton.set_visible(false);
    _fadeDownButton.set_visible(false);
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
    if (_mouseIn) {
      _fadeUpButton.set_visible(value < ControlValue::MaxUInt() * 3 / 4);
      _fadeDownButton.set_visible(value >= ControlValue::MaxUInt() * 1 / 4);
    }
    _holdUpdates = hold;
    SignalValueChange().emit(_scale.get_value());
  } else {
    _fadeUpButton.set_visible(false);
    _fadeDownButton.set_visible(false);
  }
}

void FaderWidget::Toggle() {
  _onCheckButton.SetActive(!_onCheckButton.GetActive());
}

void FaderWidget::FullOn() { _scale.set_value(ControlValue::MaxUInt()); }

void FaderWidget::FullOff() { _scale.set_value(0); }

void FaderWidget::onFadeUp() { setTargetValue(ControlValue::MaxUInt()); }

void FaderWidget::onFadeDown() { setTargetValue(0); }

void FaderWidget::ShowFadeButtons(bool mouse_in) {
  if (mouse_in != _mouseIn) {
    _mouseIn = mouse_in;
    if (mouse_in && GetSourceValue() != nullptr) {
      const double value = _scale.get_value();
      _fadeUpButton.set_visible(value < ControlValue::MaxUInt() * 3 / 4);
      _fadeDownButton.set_visible(value >= ControlValue::MaxUInt() * 1 / 4);
    } else {
      _fadeUpButton.set_visible(false);
      _fadeDownButton.set_visible(false);
    }
  }
}

}  // namespace glight::gui
