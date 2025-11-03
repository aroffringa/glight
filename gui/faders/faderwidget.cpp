#include "faderwidget.h"

#include "controlmenu.h"
#include "faderwindow.h"

#include <gtkmm/adjustment.h>
#include <gtkmm/eventcontrollerfocus.h>
#include <gtkmm/gestureclick.h>

#include "../state/faderstate.h"

#include "../../theatre/controllable.h"
#include "../../theatre/fixturecontrol.h"
#include "../../theatre/presetvalue.h"
#include "../../theatre/sourcevalue.h"

#include "../../system/uniquewithoutordering.h"

namespace glight::gui {

using theatre::ControlValue;

FaderWidget::FaderWidget(FaderWindow &fader_window, FaderState &state,
                         ControlMode mode, char key)
    : ControlWidget(fader_window, state, mode),
      _scale(Gtk::Adjustment::create(
                 0, 0, ControlValue::MaxUInt() + ControlValue::MaxUInt() / 100,
                 (ControlValue::MaxUInt() + 1) / 100),
             Gtk::Orientation::VERTICAL),
      flash_button_label_(std::string(1, key)) {
  set_orientation(Gtk::Orientation::VERTICAL);
  if (GetMode() == ControlMode::Primary) {
    _fadeUpButton.set_image_from_icon_name("go-up");
    _fadeUpButton.signal_clicked().connect([&]() { onFadeUp(); });
    _fadeUpButton.set_halign(Gtk::Align::CENTER);
    _fadeUpButton.set_valign(Gtk::Align::START);
    _fadeUpButton.set_margin_top(10);
    _overlay.add_overlay(_fadeUpButton);
  }

  auto right_press = [&](int buttons, double x, double y) {
    return HandleRightPress(buttons, x, y);
  };
  auto right_release = [&](int buttons, double x, double y) {
    return HandleRightRelease(buttons, x, y);
  };

  _scale.set_inverted(true);
  _scale.set_draw_value(false);
  _scale.set_vexpand(true);
  _scale.signal_value_changed().connect(
      sigc::mem_fun(*this, &FaderWidget::onScaleChange));
  auto scale_gesture = Gtk::GestureClick::create();
  scale_gesture->signal_pressed().connect(right_press, false);
  _scale.add_controller(scale_gesture);
  _overlay.set_child(_scale);
  _scale.show();

  auto overlay_focus = Gtk::EventControllerFocus::create();
  overlay_focus->signal_enter().connect([&]() { ShowFadeButtons(true); },
                                        false);
  overlay_focus->signal_leave().connect([&]() { ShowFadeButtons(false); },
                                        false);
  _overlay.add_controller(overlay_focus);
  auto overlay_gesture = Gtk::GestureClick::create();
  overlay_gesture->set_button(3);
  overlay_gesture->signal_pressed().connect(right_press, false);
  overlay_gesture->signal_released().connect(right_release, false);
  _overlay.add_controller(overlay_gesture);

  append(_overlay);
  _overlay.show();

  if (GetMode() == ControlMode::Primary) {
    _fadeDownButton.set_image_from_icon_name("go-down");
    _fadeDownButton.signal_clicked().connect([&]() { onFadeDown(); });
    _fadeDownButton.set_halign(Gtk::Align::CENTER);
    _fadeDownButton.set_valign(Gtk::Align::END);
    _fadeDownButton.set_margin_bottom(10);
    _overlay.add_overlay(_fadeDownButton);

    // The flash button label is added manually because it takes less space
    // like this.
    _flashButton.set_child(flash_button_label_);
    flash_button_label_.show();
    flash_button_label_.set_hexpand(false);
    auto flash_gesture = Gtk::GestureClick::create();
    flash_gesture->signal_pressed().connect(
        sigc::mem_fun(*this, &FaderWidget::onFlashButtonPressed), false);
    flash_gesture->signal_released().connect(
        sigc::mem_fun(*this, &FaderWidget::onFlashButtonReleased), false);
    _flashButton.add_controller(flash_gesture);
    append(_flashButton);
    _flashButton.set_visible(state.DisplayFlashButton());
    _flashButton.set_hexpand(false);
  }

  _checkButton.set_halign(Gtk::Align::CENTER);
  _checkButton.SignalChanged().connect(
      sigc::mem_fun(*this, &FaderWidget::onOnButtonClicked));
  append(_checkButton);
  _checkButton.set_visible(state.DisplayCheckButton());

  auto label_gesture = Gtk::GestureClick::create();
  label_gesture->signal_pressed().connect(
      [&](int, double, double) { ShowAssignDialog(); });
  _nameLabel.add_controller(label_gesture);
  _nameLabel.set_visible(state.DisplayName());

  update_display_settings_connection_ =
      State().SignalChange().connect([&]() { UpdateDisplaySettings(); });
}

FaderWidget::~FaderWidget() {
  update_display_settings_connection_.disconnect();
}

void FaderWidget::onOnButtonClicked() {
  if (!_holdUpdates) {
    _holdUpdates = true;
    if (_checkButton.GetActive())
      _scale.set_value(ControlValue::MaxUInt());
    else
      _scale.set_value(0);
    _holdUpdates = false;

    setImmediateValue(0, _scale.get_value());
    SignalValueChange().emit();
  }
}

void FaderWidget::onFlashButtonPressed(int buttons, double x, double y) {
  _scale.set_value(ControlValue::MaxUInt());
}

void FaderWidget::onFlashButtonReleased(int buttons, double x, double y) {
  _scale.set_value(0);
}

void FaderWidget::onScaleChange() {
  if (!_holdUpdates) {
    _holdUpdates = true;
    const double value = _scale.get_value();
    _checkButton.SetActive(value != 0);
    if (_mouseIn && GetSourceValue(0) && State().OverlayFadeButtons()) {
      _fadeUpButton.set_visible(value < ControlValue::MaxUInt() * 3 / 4);
      _fadeDownButton.set_visible(value >= ControlValue::MaxUInt() * 1 / 4);
    }
    _holdUpdates = false;

    setImmediateValue(0, _scale.get_value());
    SignalValueChange().emit();
  }
}

void FaderWidget::OnAssigned(bool moveFader) {
  const theatre::SourceValue *source = GetSourceValue(0);
  if (source) {
    _nameLabel.set_text(source->Name());
    if (moveFader) {
      _scale.set_value(GetSingleSourceValue(0).Value().UInt());
    } else {
      setImmediateValue(0, _scale.get_value());
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
      setImmediateValue(0, _scale.get_value());
    }
  }
}

void FaderWidget::SyncFader() {
  if (GetSourceValue(0) != nullptr) {
    const bool hold = _holdUpdates;
    _holdUpdates = true;
    const unsigned value = GetSingleSourceValue(0).Value().UInt();
    _scale.set_value(value);
    _checkButton.SetActive(value != 0);
    if (_mouseIn && State().OverlayFadeButtons()) {
      _fadeUpButton.set_visible(value < ControlValue::MaxUInt() * 3 / 4);
      _fadeDownButton.set_visible(value >= ControlValue::MaxUInt() * 1 / 4);
    }
    _holdUpdates = hold;
    SignalValueChange().emit();
  } else {
    _fadeUpButton.set_visible(false);
    _fadeDownButton.set_visible(false);
  }
}

void FaderWidget::Toggle() {
  _checkButton.SetActive(!_checkButton.GetActive());
}

void FaderWidget::FlashOn() { _scale.set_value(ControlValue::MaxUInt()); }

void FaderWidget::FlashOff() { _scale.set_value(0); }

void FaderWidget::onFadeUp() { setTargetValue(0, ControlValue::MaxUInt()); }

void FaderWidget::onFadeDown() { setTargetValue(0, 0); }

void FaderWidget::ShowFadeButtons(bool mouse_in) {
  if (mouse_in != _mouseIn) {
    _mouseIn = mouse_in;
    if (mouse_in && GetSourceValue(0) != nullptr &&
        State().OverlayFadeButtons()) {
      const double value = _scale.get_value();
      _fadeUpButton.set_visible(value < ControlValue::MaxUInt() * 3 / 4);
      _fadeDownButton.set_visible(value >= ControlValue::MaxUInt() * 1 / 4);
    } else {
      _fadeUpButton.set_visible(false);
      _fadeDownButton.set_visible(false);
    }
  }
}

void FaderWidget::HandleRightPress(int, double, double) {}

void FaderWidget::HandleRightRelease(int, double, double) {
  std::unique_ptr<ControlMenu> &menu = GetFaderWindow().GetControlMenu();
  menu = std::make_unique<ControlMenu>(State());
  menu->SignalAssign().connect([&]() { ShowAssignDialog(); });
  menu->SignalToggleName().connect(
      [&]() { State().SetDisplayName(menu->DisplayName()); });
  menu->SignalToggleFlashButton().connect(
      [&]() { State().SetDisplayFlashButton(menu->DisplayFlashButton()); });
  menu->SignalToggleCheckButton().connect(
      [&]() { State().SetDisplayCheckButton(menu->DisplayCheckButton()); });
  menu->SignalToggleFadeButtons().connect(
      [&]() { State().SetOverlayFadeButtons(menu->OverlayFadeButtons()); });
  menu->popup();
}

void FaderWidget::UpdateDisplaySettings() {
  _nameLabel.set_visible(State().DisplayName());
  _flashButton.set_visible(State().DisplayFlashButton() &&
                           GetMode() == ControlMode::Primary);
  _checkButton.set_visible(State().DisplayCheckButton());
}

}  // namespace glight::gui
