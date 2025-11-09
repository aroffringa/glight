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

  left_button_.SetIconName("go-previous");
  const auto stop_pan = [&](int) { StopPan(); };
  left_button_.SetSignalButton(1);
  left_button_.SignalPress().connect([&](int) { MoveLeft(); });
  left_button_.SignalRelease().connect(stop_pan);
  grid_.attach(left_button_, 0, 1);

  right_button_.SetIconName("go-next");
  right_button_.SetSignalButton(1);
  right_button_.SignalPress().connect([&](int) { MoveRight(); });
  right_button_.SignalRelease().connect(stop_pan);
  grid_.attach(right_button_, 2, 1);

  up_button_.SetIconName("go-up");
  up_button_.SetSignalButton(1);
  const auto stop_tilt = [&](int) { StopTilt(); };
  up_button_.SignalPress().connect([&](int) { MoveUp(); });
  up_button_.SignalRelease().connect(stop_tilt);
  grid_.attach(up_button_, 1, 0);

  down_button_.SetIconName("go-down");
  down_button_.SetSignalButton(1);
  down_button_.SignalPress().connect([&](int) { MoveDown(); });
  down_button_.SignalRelease().connect(stop_tilt);
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
  if (PanIsAssigned()) {
    theatre::SingleSourceValue &value = GetSingleSourceValue(0);
    value.Set(value.Value());
  }
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
  if (TiltIsAssigned()) {
    theatre::SingleSourceValue &value = GetSingleSourceValue(1);
    value.Set(value.Value());
  }
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
      [&](bool new_value) { State().SetDisplayName(new_value); });
  menu->SignalToggleFlashButton().connect(
      [&](bool new_value) { State().SetDisplayFlashButton(new_value); });
  menu->SignalToggleCheckButton().connect(
      [&](bool new_value) { State().SetDisplayCheckButton(new_value); });
  menu->SignalToggleFadeButtons().connect(
      [&](bool new_value) { State().SetOverlayFadeButtons(new_value); });
  menu->set_parent(GetFaderWindow());
  insert_action_group("win", menu->GetActionGroup());
  menu->popup();
}

void MoverWidget::UpdateDisplaySettings() {
  name_label_.set_visible(State().DisplayName());
}

}  // namespace glight::gui
