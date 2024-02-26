#include "togglewidget.h"

#include "controlmenu.h"
#include "faderwindow.h"

#include "../state/faderstate.h"

#include "../eventtransmitter.h"

#include "../../theatre/controlvalue.h"
#include "../../theatre/management.h"
#include "../../theatre/presetvalue.h"
#include "../../theatre/sourcevalue.h"

#include "../../theatre/scenes/scene.h"

#include "../../system/uniquewithoutordering.h"

namespace glight::gui {

ToggleWidget::ToggleWidget(FaderWindow &fader_window, FaderState &state,
                           ControlMode mode, char key)
    : ControlWidget(fader_window, state, mode),
      flash_button_label_(std::string(1, key)) {
  auto right_press = [&](GdkEventButton *event) { return event->button == 3; };
  auto right_release = [&](GdkEventButton *event) {
    if (event->button == 3)
      return HandleRightRelease(event);
    else
      return false;
  };

  // The flash button label is added manually because it takes less space
  // like this.
  flash_button_.add(flash_button_label_);
  flash_button_label_.show();
  flash_button_.set_events(Gdk::BUTTON_PRESS_MASK);
  flash_button_.signal_button_press_event().connect(
      sigc::mem_fun(*this, &ToggleWidget::OnFlashButtonPressed), false);
  flash_button_.signal_button_release_event().connect(
      sigc::mem_fun(*this, &ToggleWidget::OnFlashButtonReleased), false);
  box_.pack_start(flash_button_, false, false, 0);
  flash_button_.set_valign(Gtk::ALIGN_CENTER);
  flash_button_.set_vexpand(false);
  flash_button_.show();

  fade_button_.set_events(Gdk::BUTTON_PRESS_MASK);
  fade_button_.signal_button_release_event().connect(right_release, false);
  fade_button_.signal_clicked().connect([&]() { OnFade(); });
  fade_button_.set_image_from_icon_name("go-up");
  box_.pack_start(fade_button_, false, false, 0);
  fade_button_.set_vexpand(false);
  fade_button_.show();

  icon_button_.SignalChanged().connect([&]() { OnIconClicked(); });
  icon_button_.signal_button_press_event().connect(right_press, false);
  icon_button_.signal_button_release_event().connect(right_release, false);
  box_.pack_start(icon_button_, false, false, 3);
  icon_button_.show();

  name_label_.set_halign(Gtk::ALIGN_START);
  name_label_.set_justify(Gtk::JUSTIFY_LEFT);
  event_box_.add(name_label_);
  name_label_.show();
  name_label_.set_hexpand(true);

  event_box_.set_events(Gdk::BUTTON_PRESS_MASK);
  event_box_.signal_button_press_event().connect([&](GdkEventButton *event) {
    if (event->button == 3)
      HandleRightRelease(event);
    else
      ShowAssignDialog();
    return true;
  });
  box_.pack_start(event_box_, true, true, 0);
  event_box_.show();

  add(box_);
  box_.show();

  UpdateDisplaySettings();
  update_display_settings_connection_ =
      State().SignalChange().connect([&]() { UpdateDisplaySettings(); });
}

ToggleWidget::~ToggleWidget() {
  update_display_settings_connection_.disconnect();
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

bool ToggleWidget::OnFlashButtonPressed(GdkEventButton *event) {
  if (event->button == 3) {
    return true;
  } else {
    icon_button_.SetActive(true);
    return false;
  }
}

bool ToggleWidget::OnFlashButtonReleased(GdkEventButton *event) {
  if (event->button == 3) {
    return HandleRightRelease(event);
  } else {
    icon_button_.SetActive(false);
    return false;
  }
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
      UpdateActivated(GetSingleSourceValue(*source));
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
  menu->popup_at_pointer(reinterpret_cast<const GdkEvent *>(event));
  return true;
}

void ToggleWidget::UpdateDisplaySettings() {
  name_label_.set_visible(State().DisplayName());
  flash_button_.set_visible(State().DisplayFlashButton());
  fade_button_.set_visible(State().OverlayFadeButtons());
  icon_button_.set_visible(State().DisplayCheckButton());
}

}  // namespace glight::gui
