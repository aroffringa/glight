#include "colorcontrolwidget.h"

#include "controlmenu.h"
#include "faderwindow.h"

#include "gui/state/faderstate.h"

#include "gui/dialogs/controllableselectdialog.h"

#include "gui/eventtransmitter.h"
#include "gui/instance.h"

#include "theatre/controlvalue.h"
#include "theatre/management.h"
#include "theatre/sourcevalue.h"

namespace glight::gui {

using theatre::Color;

ColorControlWidget::ColorControlWidget(FaderWindow &fader_window,
                                       FaderState &state, ControlMode mode,
                                       char key)
    : ControlWidget(fader_window, state, mode),
      color_selector_(&fader_window, false) {
  color_selector_.SignalColorChanged().connect([&]() { OnColorChanged(); });
  box_.pack_start(color_selector_);
  color_selector_.show();

  name_label_.set_halign(Gtk::ALIGN_START);
  name_label_.set_justify(Gtk::JUSTIFY_LEFT);
  event_box_.add(name_label_);
  name_label_.show();

  event_box_.set_events(Gdk::BUTTON_PRESS_MASK);
  event_box_.signal_button_press_event().connect([&](GdkEventButton *) {
    ShowAssignControllableDialog();
    return true;
  });
  event_box_.show();
  box_.pack_start(event_box_, true, true, 0);

  add(box_);
  box_.show();

  SetDefaultSourceCount(3);

  UpdateDisplaySettings();
  update_display_settings_connection_ =
      State().SignalChange().connect([&]() { UpdateDisplaySettings(); });
}

ColorControlWidget::~ColorControlWidget() {
  update_display_settings_connection_.disconnect();
}

void ColorControlWidget::OnColorChanged() {
  if (!hold_updates_) {
    const Color new_color = color_selector_.GetColor();
    const Color old_color = ColorFromSourceValues();
    if (old_color != new_color) {
      setImmediateValue(0, new_color.Red() << 16);
      setImmediateValue(1, new_color.Green() << 16);
      setImmediateValue(2, new_color.Blue() << 16);
      SignalValueChange().emit();
    }
  }
}

Color ColorControlWidget::ColorFromSourceValues() const {
  const std::vector<theatre::SourceValue *> &sources = GetSourceValues();
  const bool has_red = sources.size() > 0 && sources[0];
  const bool has_green = sources.size() > 1 && sources[1];
  const bool has_blue = sources.size() > 2 && sources[2];
  using theatre::ControlValue;
  const ControlValue red =
      has_red ? GetSingleSourceValue(0).Value() : ControlValue();
  const ControlValue green =
      has_green ? GetSingleSourceValue(1).Value() : ControlValue();
  const ControlValue blue =
      has_blue ? GetSingleSourceValue(2).Value() : ControlValue();
  return Color(red.ToUChar(), green.ToUChar(), blue.ToUChar());
}

void ColorControlWidget::OnAssigned(bool moveFader) {
  const theatre::SourceValue *source = GetSourceValue(0);
  if (source) {
    name_label_.set_text(source->GetControllable().Name());
    const theatre::Controllable *controllable = &source->GetControllable();
    const std::vector<Color> colors =
        controllable->InputColors(source->InputIndex());
    if (moveFader) {
      const Color color = ColorFromSourceValues();
      color_selector_.SetColor(color);
    } else {
      OnColorChanged();
    }
  } else {
    name_label_.set_text("<..>");
    if (moveFader) {
      color_selector_.SetColor(Color::Gray(128));
    }
  }
  if (moveFader) {
    SignalValueChange().emit();
  }
}

void ColorControlWidget::SyncFader() {
  if (IsAssigned()) {
    const Color color = ColorFromSourceValues();
    if (color != color_selector_.GetColor()) {
      color_selector_.SetColor(color);
      SignalValueChange().emit();
    }
  }
}

void ColorControlWidget::Toggle() {
  const Color color = color_selector_.GetColor();
  color_selector_.SetColor(previous_color_);
  previous_color_ = color;
}

void ColorControlWidget::FlashOn() { Toggle(); }

void ColorControlWidget::FlashOff() { Toggle(); }

void ColorControlWidget::Limit(double value) {}

bool ColorControlWidget::HandleRightRelease(GdkEventButton *event) {
  std::unique_ptr<ControlMenu> &menu = GetFaderWindow().GetControlMenu();
  menu = std::make_unique<ControlMenu>(State());
  menu->SignalAssign().connect([&]() { ShowAssignControllableDialog(); });
  menu->SignalToggleName().connect(
      [&]() { State().SetDisplayName(menu->DisplayName()); });
  menu->popup_at_pointer(reinterpret_cast<const GdkEvent *>(event));
  return true;
}

void ColorControlWidget::UpdateDisplaySettings() {
  name_label_.set_visible(State().DisplayName());
}

void ColorControlWidget::ShowAssignControllableDialog() {
  ControllableSelectDialog dialog("Select item for color control", false);
  dialog.SetFilter(ObjectListType::All);
  if (dialog.run() == Gtk::RESPONSE_OK) {
    theatre::Controllable *controllable =
        dynamic_cast<theatre::Controllable *>(dialog.SelectedObject());
    if (controllable) {
      std::vector<theatre::SourceValue *> sources(3);
      for (size_t i = 0; i != controllable->NInputs(); ++i) {
        theatre::Management &management = Instance::Management();
        if (controllable->InputType(i) == theatre::FunctionType::Red)
          sources[0] = management.GetSourceValue(*controllable, i);
        else if (controllable->InputType(i) == theatre::FunctionType::Green)
          sources[1] = management.GetSourceValue(*controllable, i);
        else if (controllable->InputType(i) == theatre::FunctionType::Blue)
          sources[2] = management.GetSourceValue(*controllable, i);
      }
      Assign(sources, true);
    }
  }
}

}  // namespace glight::gui
