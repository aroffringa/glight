#include "moverwidget.h"

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

MoverWidget::MoverWidget(FaderWindow &fader_window, FaderState &state,
                         ControlMode mode, char key)
    : ControlWidget(fader_window, state, mode) {
  SetDefaultSourceCount(2);

  left_button_.set_image_from_icon_name("go-previous");
  left_button_.set_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
  left_button_.signal_button_press_event().connect(
      [&](GdkEventButton *event) {
        if (event->button == 1) MoveLeft();
        return false;
      },
      false);
  left_button_.signal_button_release_event().connect(
      [&](GdkEventButton *event) {
        if (event->button == 1) StopPan();
        return false;
      },
      false);
  grid_.attach(left_button_, 0, 1);
  right_button_.set_image_from_icon_name("go-next");
  right_button_.set_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
  right_button_.signal_button_press_event().connect(
      [&](GdkEventButton *event) {
        if (event->button == 1) MoveRight();
        return false;
      },
      false);
  right_button_.signal_button_release_event().connect(
      [&](GdkEventButton *event) {
        if (event->button == 1) StopPan();
        return false;
      },
      false);
  grid_.attach(right_button_, 2, 1);
  up_button_.set_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
  up_button_.signal_button_press_event().connect(
      [&](GdkEventButton *event) {
        if (event->button == 1) MoveUp();
        return false;
      },
      false);
  up_button_.signal_button_release_event().connect(
      [&](GdkEventButton *event) {
        if (event->button == 1) StopTilt();
        return false;
      },
      false);
  up_button_.set_image_from_icon_name("go-up");
  grid_.attach(up_button_, 1, 0);
  down_button_.set_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
  down_button_.signal_button_press_event().connect(
      [&](GdkEventButton *event) {
        if (event->button == 1) MoveDown();
        return false;
      },
      false);
  down_button_.signal_button_release_event().connect(
      [&](GdkEventButton *event) {
        if (event->button == 1) StopTilt();
        return false;
      },
      false);
  down_button_.set_image_from_icon_name("go-down");
  grid_.attach(down_button_, 1, 2);

  name_label_.set_halign(Gtk::ALIGN_START);
  name_label_.set_justify(Gtk::JUSTIFY_LEFT);
  event_box_.add(name_label_);
  name_label_.set_hexpand(true);

  event_box_.set_events(Gdk::BUTTON_PRESS_MASK);
  event_box_.signal_button_press_event().connect([&](GdkEventButton *event) {
    if (event->button == 3)
      HandleRightRelease(event);
    else
      ShowAssignDialog();
    return true;
  });
  grid_.attach(event_box_, 0, 3, 3, 1);

  add(grid_);
  grid_.show_all();

  UpdateDisplaySettings();
  update_display_settings_connection_ =
      State().SignalChange().connect([&]() { UpdateDisplaySettings(); });
}

void MoverWidget::MoveLeft() {
  SetFadeDownSpeed(0.1);
  SetFadeUpSpeed(0.1);
  setTargetValue(0, 0);
}

void MoverWidget::MoveRight() {
  SetFadeDownSpeed(0.1);
  SetFadeUpSpeed(0.1);
  setTargetValue(0, theatre::ControlValue::MaxUInt());
}

void MoverWidget::StopPan() {
  theatre::SingleSourceValue &value = GetSingleSourceValue(0);
  value.Set(value.Value());
}

void MoverWidget::MoveUp() {
  SetFadeDownSpeed(0.1);
  SetFadeUpSpeed(0.1);
  setTargetValue(1, theatre::ControlValue::MaxUInt());
}

void MoverWidget::MoveDown() {
  SetFadeDownSpeed(0.1);
  SetFadeUpSpeed(0.1);
  setTargetValue(1, 0);
}

void MoverWidget::StopTilt() {
  theatre::SingleSourceValue &value = GetSingleSourceValue(1);
  value.Set(value.Value());
}

void MoverWidget::OnAssigned(bool move_fader) {
  std::string name;
  if (theatre::SourceValue *pan_source = GetSourceValue(0); pan_source) {
    name = pan_source->Name() + "\n";
  } else {
    name = "<..>\n";
  }
  if (theatre::SourceValue *tilt_source = GetSourceValue(1); tilt_source) {
    name += tilt_source->Name();
  } else {
    name += "<..>";
  }
  name_label_.set_text(name);
}

bool MoverWidget::HandleRightRelease(GdkEventButton *event) {
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

void MoverWidget::UpdateDisplaySettings() {
  name_label_.set_visible(State().DisplayName());
}

}  // namespace glight::gui
