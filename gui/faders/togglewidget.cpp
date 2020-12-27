#include "togglewidget.h"

#include "../eventtransmitter.h"

#include "../dialogs/inputselectdialog.h"

#include "../../theatre/controlvalue.h"
#include "../../theatre/management.h"
#include "../../theatre/presetvalue.h"

ToggleWidget::ToggleWidget(class Management &management,
                           EventTransmitter &eventHub, char key)
    : _flashButton(std::string(1, key)), _nameLabel("<..>"),
      _management(&management), _eventHub(eventHub), _preset(nullptr),
      _holdUpdates(false) {
  _updateConnection =
      _eventHub.SignalUpdateControllables().connect([&]() { onUpdate(); });

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

ToggleWidget::~ToggleWidget() { _updateConnection.disconnect(); }

void ToggleWidget::onOnButtonClicked() {
  if (!_holdUpdates) {
    unsigned value;
    if (_onCheckButton.get_active())
      value = ControlValue::MaxUInt();
    else
      value = 0;

    writeValue(value);
    SignalValueChange().emit(value);
  }
}

bool ToggleWidget::onFlashButtonPressed(GdkEventButton *event) {
  _onCheckButton.set_active(true);
  return false;
}

bool ToggleWidget::onFlashButtonReleased(GdkEventButton *event) {
  _onCheckButton.set_active(false);
  return false;
}

bool ToggleWidget::onNameLabelClicked(GdkEventButton *event) {
  InputSelectDialog dialog(*_management, _eventHub);
  if (dialog.run() == Gtk::RESPONSE_OK) {
    Assign(dialog.SelectedInputPreset(), true);
  }
  return true;
}

void ToggleWidget::Assign(PresetValue *item, bool moveFader) {
  if (item != _preset) {
    _preset = item;
    if (_preset != nullptr) {
      _nameLabel.set_text(_preset->Title());
      if (moveFader) {
        immediateAssign(_preset->Value().UInt());
        _onCheckButton.set_active(_preset->Value().UInt() != 0);
      } else {
        if (_onCheckButton.get_active())
          writeValue(ControlValue::MaxUInt());
        else
          writeValue(0);
      }
    } else {
      _nameLabel.set_text("<..>");
      if (moveFader) {
        immediateAssign(0);
        _onCheckButton.set_active(false);
      } else {
        if (_onCheckButton.get_active())
          writeValue(ControlValue::MaxUInt());
        else
          writeValue(0);
      }
    }
    SignalAssigned().emit();
    if (moveFader) {
      unsigned value;
      if (_onCheckButton.get_active())
        value = ControlValue::MaxUInt();
      else
        value = 0;
      SignalValueChange().emit(value);
    }
  }
}

void ToggleWidget::MoveSlider() {
  if (_preset != nullptr) {
    immediateAssign(_preset->Value().UInt());
    _onCheckButton.set_active(_preset->Value().UInt() != 0);
    SignalValueChange().emit(_preset->Value().UInt());
  }
}

void ToggleWidget::onUpdate() {
  if (_preset != nullptr) {
    // The preset might be removed, if so update label
    if (!_management->Contains(*_preset)) {
      _nameLabel.set_text("<..>");
      _preset = nullptr;
      _onCheckButton.set_active(false);
    }
    // Only if not removed: if preset is renamed, update
    else {
      _nameLabel.set_text(_preset->Title());
    }
  }
}

void ToggleWidget::Toggle() {
  _onCheckButton.set_active(!_onCheckButton.get_active());
}

void ToggleWidget::FullOn() { _onCheckButton.set_active(true); }

void ToggleWidget::FullOff() { _onCheckButton.set_active(false); }

void ToggleWidget::ChangeManagement(class Management &management,
                                    bool moveSliders) {
  if (_preset == nullptr) {
    _management = &management;
  } else {
    std::string controllablePath = _preset->Controllable().FullPath();
    size_t input = _preset->InputIndex();
    _management = &management;
    Controllable &controllable = static_cast<Controllable &>(
        _management->GetObjectFromPath(controllablePath));
    PresetValue *pv = _management->GetPresetValue(controllable, input);
    if (pv == nullptr)
      Unassign();
    else {
      Assign(pv, moveSliders);
    }
  }
}

void ToggleWidget::Limit(double value) {
  if (value < ControlValue::MaxUInt())
    _onCheckButton.set_active(false);
}
