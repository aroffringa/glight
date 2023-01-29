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

  auto right_press = [&](GdkEventButton *event) {
    return HandleRightPress(event);
  };
  auto right_release = [&](GdkEventButton *event) {
    return HandleRightRelease(event);
  };

  _scale.set_inverted(true);
  _scale.set_draw_value(false);
  _scale.set_vexpand(true);
  _scale.signal_value_changed().connect(
      sigc::mem_fun(*this, &FaderWidget::onScaleChange));
  _scale.signal_button_press_event().connect(right_press, false);
  _overlay.add(_scale);
  _scale.show();

  _mouseInBox.set_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK |
                         Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
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
  _mouseInBox.signal_button_press_event().connect(right_press, false);
  _mouseInBox.signal_button_release_event().connect(right_release, false);

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

  _checkButton.set_halign(Gtk::ALIGN_CENTER);
  _checkButton.SignalChanged().connect(
      sigc::mem_fun(*this, &FaderWidget::onOnButtonClicked));
  _box.pack_start(_checkButton, false, false, 0);
  _checkButton.show();

  _labelEventBox.set_events(Gdk::BUTTON_PRESS_MASK);
  _labelEventBox.show();

  _labelEventBox.signal_button_press_event().connect([&](GdkEventButton *) {
    ShowAssignDialog();
    return true;
  });
  _labelEventBox.add(_nameLabel);
  _nameLabel.show();

  add(_box);
  _box.show();

  MakePopupMenu();
}

void FaderWidget::MakePopupMenu() {
  _miAssign.signal_activate().connect([&]() { ShowAssignDialog(); });
  _menu.append(_miAssign);
  _menu.append(_miSeperator1);
  _miDisplayLabel.set_active(true);
  _miDisplayLabel.signal_activate().connect(
      [&]() { _nameLabel.set_visible(_miDisplayLabel.get_active()); });
  _menu.append(_miDisplayLabel);
  _miDisplayFlashButton.set_active(true);
  _miDisplayFlashButton.signal_activate().connect(
      [&]() { _flashButton.set_visible(_miDisplayFlashButton.get_active()); });
  _menu.append(_miDisplayFlashButton);
  _miDisplayCheckButton.set_active(true);
  _miDisplayCheckButton.signal_activate().connect(
      [&]() { _checkButton.set_visible(_miDisplayCheckButton.get_active()); });
  _menu.append(_miDisplayCheckButton);
  _miOverlayFadeButtons.set_active(true);
  _menu.append(_miOverlayFadeButtons);

  _menu.show_all_children();
}

void FaderWidget::onOnButtonClicked() {
  if (!_holdUpdates) {
    _holdUpdates = true;
    if (_checkButton.GetActive())
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
    _checkButton.SetActive(value != 0);
    if (_mouseIn && GetSourceValue() != nullptr &&
        _miOverlayFadeButtons.get_active()) {
      _fadeUpButton.set_visible(value < ControlValue::MaxUInt() * 3 / 4);
      _fadeDownButton.set_visible(value >= ControlValue::MaxUInt() * 1 / 4);
    }
    _holdUpdates = false;

    setImmediateValue(_scale.get_value());
    SignalValueChange().emit(_scale.get_value());
  }
}

void FaderWidget::ShowAssignDialog() {
  InputSelectDialog dialog(GetManagement(), GetEventHub());
  if (dialog.run() == Gtk::RESPONSE_OK) {
    Assign(dialog.SelectedInputPreset(), true);
  }
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
    _checkButton.SetColors(UniqueWithoutOrdering(colors));
  } else {
    _nameLabel.set_text("<..>");
    _checkButton.SetColors({});
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
    _checkButton.SetActive(value != 0);
    if (_mouseIn && _miOverlayFadeButtons.get_active()) {
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
  _checkButton.SetActive(!_checkButton.GetActive());
}

void FaderWidget::FullOn() { _scale.set_value(ControlValue::MaxUInt()); }

void FaderWidget::FullOff() { _scale.set_value(0); }

void FaderWidget::onFadeUp() { setTargetValue(ControlValue::MaxUInt()); }

void FaderWidget::onFadeDown() { setTargetValue(0); }

void FaderWidget::ShowFadeButtons(bool mouse_in) {
  if (mouse_in != _mouseIn) {
    _mouseIn = mouse_in;
    if (mouse_in && GetSourceValue() != nullptr &&
        _miOverlayFadeButtons.get_active()) {
      const double value = _scale.get_value();
      _fadeUpButton.set_visible(value < ControlValue::MaxUInt() * 3 / 4);
      _fadeDownButton.set_visible(value >= ControlValue::MaxUInt() * 1 / 4);
    } else {
      _fadeUpButton.set_visible(false);
      _fadeDownButton.set_visible(false);
    }
  }
}

bool FaderWidget::HandleRightPress(GdkEventButton *event) {
  return event->button == 3;  // right button?
}

bool FaderWidget::HandleRightRelease(GdkEventButton *event) {
  if (event->button == 3) {  // right button?
    _menu.popup(event->button, event->time);
    return true;
  }
  return false;
}

}  // namespace glight::gui
