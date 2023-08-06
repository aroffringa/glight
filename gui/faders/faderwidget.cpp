#include "faderwidget.h"

#include "controlmenu.h"
#include "faderwindow.h"

#include <gtkmm/adjustment.h>

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
             Gtk::ORIENTATION_VERTICAL),
      _flashButton(std::string(1, key)) {
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
    _flashButton.set_visible(state.DisplayFlashButton());
  }

  _checkButton.set_halign(Gtk::ALIGN_CENTER);
  _checkButton.SignalChanged().connect(
      sigc::mem_fun(*this, &FaderWidget::onOnButtonClicked));
  _box.pack_start(_checkButton, false, false, 0);
  _checkButton.set_visible(state.DisplayCheckButton());

  _labelEventBox.set_events(Gdk::BUTTON_PRESS_MASK);
  _labelEventBox.show();

  _labelEventBox.signal_button_press_event().connect([&](GdkEventButton *) {
    ShowAssignDialog();
    return true;
  });
  _labelEventBox.add(_nameLabel);
  _nameLabel.set_visible(state.DisplayName());

  add(_box);
  _box.show();

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
        State().OverlayFadeButtons()) {
      _fadeUpButton.set_visible(value < ControlValue::MaxUInt() * 3 / 4);
      _fadeDownButton.set_visible(value >= ControlValue::MaxUInt() * 1 / 4);
    }
    _holdUpdates = false;

    setImmediateValue(_scale.get_value());
    SignalValueChange().emit(_scale.get_value());
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
    if (_mouseIn && State().OverlayFadeButtons()) {
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

bool FaderWidget::HandleRightPress(GdkEventButton *event) {
  return event->button == 3;  // right button?
}

bool FaderWidget::HandleRightRelease(GdkEventButton *event) {
  if (event->button == 3) {  // right button?
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
    menu->popup_at_pointer(reinterpret_cast<const GdkEvent *>(event));
    return true;
  }
  return false;
}

void FaderWidget::UpdateDisplaySettings() {
  _nameLabel.set_visible(State().DisplayName());
  _flashButton.set_visible(State().DisplayFlashButton());
  _checkButton.set_visible(State().DisplayCheckButton());
}

}  // namespace glight::gui
