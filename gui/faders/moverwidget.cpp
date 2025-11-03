#include "moverwidget.h"

#include <gtkmm/gestureclick.h>

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
  auto left_gesture = Gtk::GestureClick::create();
  const auto stop_pan = [&](int, double, double) { StopPan(); };
  left_gesture->set_button(1);
  left_gesture->signal_pressed().connect(
      [&](int, double, double) { MoveLeft(); });
  left_gesture->signal_released().connect(stop_pan);
  left_button_.add_controller(left_gesture);
  grid_.attach(left_button_, 0, 1);

  right_button_.set_image_from_icon_name("go-next");
  auto right_gesture = Gtk::GestureClick::create();
  right_gesture->set_button(1);
  right_gesture->signal_pressed().connect(
      [&](int, double, double) { MoveRight(); });
  right_gesture->signal_released().connect(stop_pan);
  grid_.attach(right_button_, 2, 1);

  up_button_.set_image_from_icon_name("go-up");
  auto up_gesture = Gtk::GestureClick::create();
  up_gesture->set_button(1);
  const auto stop_tilt = [&](int, double, double) { StopPan(); };
  up_gesture->signal_pressed().connect([&](int, double, double) { MoveUp(); });
  up_gesture->signal_released().connect(stop_tilt);
  up_button_.add_controller(up_gesture);
  grid_.attach(up_button_, 1, 0);

  down_button_.set_image_from_icon_name("go-down");
  auto down_gesture = Gtk::GestureClick::create();
  down_gesture->set_button(1);
  down_gesture->signal_pressed().connect(
      [&](int, double, double) { MoveDown(); });
  down_gesture->signal_released().connect(stop_tilt);
  down_button_.add_controller(down_gesture);
  grid_.attach(down_button_, 1, 2);

  auto label_gesture = Gtk::GestureClick::create();
  name_label_.set_halign(Gtk::Align::START);
  name_label_.set_justify(Gtk::Justification::LEFT);
  name_label_.set_hexpand(true);
  label_gesture->set_button(0);
  label_gesture->signal_pressed().connect(
      [this, g = label_gesture.get()](int, double, double) {
        if (g->get_current_button() == 3)
          HandleRightRelease();
        else
          ShowAssignDialog();
      });
  name_label_.add_controller(label_gesture);
  grid_.attach(name_label_, 0, 3, 3, 1);

  append(grid_);

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

void MoverWidget::HandleRightRelease() {
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

void MoverWidget::UpdateDisplaySettings() {
  name_label_.set_visible(State().DisplayName());
}

}  // namespace glight::gui
