#include "togglewidget.h"

#include <gtkmm/gestureclick.h>

#include "controlmenu.h"
#include "faderwindow.h"

#include "gui/eventtransmitter.h"

#include "uistate/faderstate.h"

#include "theatre/controlvalue.h"
#include "theatre/management.h"
#include "theatre/presetvalue.h"
#include "theatre/sourcevalue.h"

#include "theatre/scenes/scene.h"

#include "system/uniquewithoutordering.h"

namespace glight::gui {

ToggleWidget::ToggleWidget(FaderWindow &fader_window,
                           uistate::FaderState &state, ControlMode mode,
                           char key)
    : ControlWidget(fader_window, state, mode),
      flash_button_(std::string(1, key)) {
  auto gesture = Gtk::GestureClick::create();
  gesture->set_button(3);

  flash_button_.SetSignalButton(1);
  flash_button_.SignalPress().connect(
      [this](int button) { OnFlashButtonPressed(button); });
  flash_button_.SignalRelease().connect(
      [this](int button) { OnFlashButtonReleased(button); });
  append(flash_button_);

  fade_button_.set_image_from_icon_name("go-up");
  fade_button_.signal_clicked().connect([&]() { OnFade(); });
  append(fade_button_);
  fade_button_.set_vexpand(false);

  icon_button_.SignalChanged().connect([&]() { OnIconClicked(); });
  append(icon_button_);

  auto label_gesture = Gtk::GestureClick::create();
  name_label_.set_halign(Gtk::Align::START);
  name_label_.set_justify(Gtk::Justification::LEFT);
  name_label_.set_hexpand(true);
  label_gesture->set_button(1);
  label_gesture->signal_pressed().connect(
      [this, g = label_gesture.get()](int, double, double) {
        ShowAssignDialog();
      });
  name_label_.add_controller(label_gesture);

  append(name_label_);

  UpdateDisplaySettings();
  update_display_settings_connection_ =
      State().SignalChange().connect([&]() { UpdateDisplaySettings(); });
}

void ToggleWidget::OnIconClicked() {
  if (!hold_updates_) {
    unsigned value;
    if (icon_button_.GetActive())
      value = theatre::ControlValue::MaxUInt();
    else
      value = 0;

    setImmediateValue(0, value);
    SignalValueChange().emit();
  }
}

void ToggleWidget::OnFlashButtonPressed(int button) {
  icon_button_.SetActive(true);
}

void ToggleWidget::OnFlashButtonReleased(int button) {
  icon_button_.SetActive(false);
}

void ToggleWidget::OnFade() {
  if (icon_button_.GetActive()) {
    setTargetValue(0, 0);
  } else {
    setTargetValue(0, theatre::ControlValue::MaxUInt());
  }
}

void ToggleWidget::UpdateActivated(const theatre::SingleSourceValue &value) {
  if (!hold_updates_) {
    hold_updates_ = true;
    icon_button_.SetActive(static_cast<bool>(value.Value()));
    if (value.TargetValue() != 0)
      fade_button_.set_image_from_icon_name("go-down");
    else
      fade_button_.set_image_from_icon_name("go-up");
    hold_updates_ = false;
  }
}

void ToggleWidget::OnAssigned(bool moveFader) {
  theatre::SourceValue *source = GetSourceValue(0);
  if (source) {
    name_label_.set_text(source->Name());
    const theatre::Controllable *controllable = &source->GetControllable();
    const std::vector<theatre::Color> colors =
        controllable->InputColors(source->InputIndex());
    icon_button_.SetColors(UniqueWithoutOrdering(colors));
    if (moveFader) {
      UpdateActivated(GetSingleSourceValue(*source));
    } else {
      if (icon_button_.GetActive())
        setTargetValue(0, theatre::ControlValue::MaxUInt());
      else
        setTargetValue(0, 0);
    }
  } else {
    name_label_.set_text("<..>");
    icon_button_.SetColors({});
    if (moveFader) {
      hold_updates_ = true;
      icon_button_.SetActive(false);
      fade_button_.set_image_from_icon_name("go-down");
      hold_updates_ = false;
    } else {
      if (icon_button_.GetActive())
        setTargetValue(0, theatre::ControlValue::MaxUInt());
      else
        setTargetValue(0, 0);
    }
  }
  if (moveFader) {
    SignalValueChange().emit();
  }
}

void ToggleWidget::SyncFader() {
  theatre::SourceValue *source = GetSourceValue(0);
  if (source != nullptr) {
    theatre::SingleSourceValue &source_value = GetSingleSourceValue(*source);
    // Most sliders will be off, so as a small optimization test this before
    // doing the more expensive dynamic_cast
    if (source_value.Value()) {
      const theatre::Scene *scene =
          dynamic_cast<const theatre::Scene *>(&source->GetControllable());
      // If the control is connected to a scene, uncheck control if scene has
      // finished
      if (scene && !scene->IsPlaying()) {
        // Build a slight delay in, so that a control isn't turned off just
        // because the management thread hasn't started the scene yet.
        ++counter_;
        if (counter_ > 5) {
          source_value.Set(0);
          counter_ = 0;
        }
      }
    }
    UpdateActivated(source_value);
    SignalValueChange().emit();
  }
}

void ToggleWidget::Toggle() {
  icon_button_.SetActive(!icon_button_.GetActive());
}

void ToggleWidget::FlashOn() { icon_button_.SetActive(true); }

void ToggleWidget::FlashOff() { icon_button_.SetActive(false); }

void ToggleWidget::Limit(double value) {
  if (value < theatre::ControlValue::MaxUInt()) icon_button_.SetActive(false);
}

void ToggleWidget::UpdateDisplaySettings() {
  name_label_.set_visible(State().DisplayName());
  flash_button_.set_visible(State().DisplayFlashButton());
  fade_button_.set_visible(State().OverlayFadeButtons());
  icon_button_.set_visible(State().DisplayCheckButton());
}

}  // namespace glight::gui
