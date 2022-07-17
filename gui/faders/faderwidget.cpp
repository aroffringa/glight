#include "faderwidget.h"

#include "../eventtransmitter.h"

#include "../dialogs/inputselectdialog.h"

#include "../../theatre/controllable.h"
#include "../../theatre/management.h"
#include "../../theatre/presetvalue.h"
#include "../../theatre/sourcevalue.h"

namespace glight::gui {

using theatre::ControlValue;

FaderWidget::FaderWidget(theatre::Management &management,
                         EventTransmitter &eventHub, char key)
    : _scale(0, ControlValue::MaxUInt() + ControlValue::MaxUInt() / 100,
             (ControlValue::MaxUInt() + 1) / 100),
      _flashButton(std::string(1, key)),
      _nameLabel("<..>"),
      _management(&management),
      _eventHub(eventHub),
      _sourceValue(nullptr),
      _holdUpdates(false) {
  _updateConnection =
      _eventHub.SignalUpdateControllables().connect([&]() { onUpdate(); });

  _scale.set_inverted(true);
  _scale.set_draw_value(false);
  _scale.set_vexpand(true);
  _scale.signal_value_changed().connect(
      sigc::mem_fun(*this, &FaderWidget::onScaleChange));
  _box.pack_start(_scale, true, true, 0);
  _scale.show();

  _flashButton.set_events(Gdk::BUTTON_PRESS_MASK);
  _flashButton.signal_button_press_event().connect(
      sigc::mem_fun(*this, &FaderWidget::onFlashButtonPressed), false);
  _flashButton.set_events(Gdk::BUTTON_PRESS_MASK);
  _flashButton.signal_button_release_event().connect(
      sigc::mem_fun(*this, &FaderWidget::onFlashButtonReleased), false);
  _box.pack_start(_flashButton, false, false, 0);
  _flashButton.show();

  _onCheckButton.set_halign(Gtk::ALIGN_CENTER);
  _onCheckButton.signal_clicked().connect(
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

FaderWidget::~FaderWidget() { _updateConnection.disconnect(); }

void FaderWidget::onOnButtonClicked() {
  if (!_holdUpdates) {
    _holdUpdates = true;
    if (_onCheckButton.get_active())
      _scale.set_value(ControlValue::MaxUInt());
    else
      _scale.set_value(0);
    _holdUpdates = false;

    setValue(_scale.get_value());
    SignalValueChange().emit(_scale.get_value());
  }
}

bool FaderWidget::onFlashButtonPressed(GdkEventButton *event) {
  _scale.set_value(ControlValue::MaxUInt());
  return false;
}

bool FaderWidget::onFlashButtonReleased(GdkEventButton *event) {
  _scale.set_value(0);
  return false;
}

void FaderWidget::onScaleChange() {
  if (!_holdUpdates) {
    _holdUpdates = true;
    _onCheckButton.set_active(_scale.get_value() != 0);
    _holdUpdates = false;

    setValue(_scale.get_value());
    SignalValueChange().emit(_scale.get_value());
  }
}

bool FaderWidget::onNameLabelClicked(GdkEventButton *event) {
  InputSelectDialog dialog(*_management, _eventHub);
  if (dialog.run() == Gtk::RESPONSE_OK) {
    Assign(dialog.SelectedInputPreset(), true);
  }
  return true;
}

void FaderWidget::Assign(theatre::SourceValue *item, bool moveFader) {
  if (item != _sourceValue) {
    _sourceValue = item;
    if (_sourceValue != nullptr) {
      _nameLabel.set_text(_sourceValue->Preset().Title());
      if (moveFader) {
        _scale.set_value(_sourceValue->Preset().Value().UInt());
      } else
        setValue(_scale.get_value());
    } else {
      _nameLabel.set_text("<..>");
      if (moveFader) {
        _scale.set_value(0);
      } else
        setValue(_scale.get_value());
    }
    SignalAssigned().emit();
    if (moveFader) SignalValueChange().emit(_scale.get_value());
  }
}

void FaderWidget::MoveSlider() {
  if (_sourceValue != nullptr) {
    _scale.set_value(_sourceValue->Preset().Value().UInt());
    SignalValueChange().emit(_scale.get_value());
  }
}

void FaderWidget::onUpdate() {
  if (_sourceValue != nullptr) {
    // The preset might be removed, if so update label
    if (!_management->Contains(*_sourceValue)) {
      _nameLabel.set_text("<..>");
      _sourceValue = nullptr;
      _scale.set_value(0.0);
    }
    // Only if not removed: if preset is renamed, update
    else {
      _nameLabel.set_text(_sourceValue->Preset().Title());
    }
  }
}

void FaderWidget::Toggle() {
  _onCheckButton.set_active(!_onCheckButton.get_active());
}

void FaderWidget::FullOn() { _scale.set_value(ControlValue::MaxUInt()); }

void FaderWidget::FullOff() { _scale.set_value(0); }

void FaderWidget::ChangeManagement(theatre::Management &management,
                                   bool moveSliders) {
  if (_sourceValue == nullptr) {
    _management = &management;
  } else {
    std::string controllablePath = _sourceValue->GetControllable().FullPath();
    size_t input = _sourceValue->Preset().InputIndex();
    _management = &management;
    theatre::Controllable *controllable = dynamic_cast<theatre::Controllable *>(
        _management->GetObjectFromPathIfExists(controllablePath));
    theatre::SourceValue *sv;
    if (controllable)
      sv = _management->GetSourceValue(*controllable, input);
    else
      sv = nullptr;
    if (sv == nullptr)
      Unassign();
    else {
      Assign(sv, moveSliders);
    }
  }
}

}  // namespace glight::gui
