#include "togglewidget.h"

#include "controlmenu.h"
#include "faderwindow.h"

#include "../state/faderstate.h"

#include "../eventtransmitter.h"

#include "../../theatre/controlvalue.h"
#include "../../theatre/management.h"
#include "../../theatre/presetvalue.h"
#include "../../theatre/sourcevalue.h"

#include "../../system/uniquewithoutordering.h"

namespace glight::gui {

ToggleWidget::ToggleWidget(FaderWindow &fader_window, FaderState &state,
                           ControlMode mode, char key)
    : ControlWidget(fader_window, state, mode),
      _flashButton(std::string(1, key)),
      _nameLabel("<..>"),
      _holdUpdates(false) {
  auto right_press = [&](GdkEventButton *event) { return event->button == 3; };
  auto right_release = [&](GdkEventButton *event) {
    if (event->button == 3)
      return HandleRightRelease(event);
    else
      return false;
  };

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
  _iconButton.signal_button_press_event().connect(right_press, false);
  _iconButton.signal_button_release_event().connect(right_release, false);
  _box.pack_start(_iconButton, false, false, 3);
  _iconButton.show();

  _nameLabel.set_halign(Gtk::ALIGN_START);
  _nameLabel.set_justify(Gtk::JUSTIFY_LEFT);
  _eventBox.add(_nameLabel);
  _nameLabel.show();

  _eventBox.set_events(Gdk::BUTTON_PRESS_MASK);
  _eventBox.signal_button_press_event().connect([&](GdkEventButton *) {
    ToggleWidget::ShowAssignDialog();
    return true;
  });
  _eventBox.show();
  _box.pack_start(_eventBox, true, true, 0);

  add(_box);
  _box.show();

  update_display_settings_connection_ =
      State().SignalChange().connect([&]() { UpdateDisplaySettings(); });
}

ToggleWidget::~ToggleWidget() {
  update_display_settings_connection_.disconnect();
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

bool ToggleWidget::onFlashButtonPressed(GdkEventButton *event) {
  if (event->button == 3) {
    return true;
  } else {
    _iconButton.SetActive(true);
    return false;
  }
}

bool ToggleWidget::onFlashButtonReleased(GdkEventButton *event) {
  if (event->button == 3) {
    return HandleRightRelease(event);
  } else {
    _iconButton.SetActive(false);
    return false;
  }
}

void ToggleWidget::OnAssigned(bool moveFader) {
  if (GetSourceValue() != nullptr) {
    _nameLabel.set_text(GetSourceValue()->Name());
    const theatre::Controllable *controllable =
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

bool ToggleWidget::HandleRightRelease(GdkEventButton *event) {
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
  menu->popup(event->button, event->time);
  return true;
}

void ToggleWidget::UpdateDisplaySettings() {
  _nameLabel.set_visible(State().DisplayName());
  _flashButton.set_visible(State().DisplayFlashButton());
  _iconButton.set_visible(State().DisplayCheckButton());
}

}  // namespace glight::gui
