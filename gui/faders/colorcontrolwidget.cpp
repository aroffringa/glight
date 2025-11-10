#include "colorcontrolwidget.h"

#include "controlmenu.h"
#include "faderwindow.h"

#include "gui/state/faderstate.h"

#include "gui/dialogs/controllableselectiondialog.h"

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
  append(color_selector_);
  color_selector_.show();

  name_label_.set_halign(Gtk::Align::START);
  name_label_.set_justify(Gtk::Justification::LEFT);
  auto gesture = Gtk::GestureClick::create();
  gesture->signal_pressed().connect(
      [&](int, double, double) { ShowAssignControllableDialog(); });
  name_label_.add_controller(gesture);
  append(name_label_);

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

void ColorControlWidget::PrepareContextMenu(ControlMenu &menu) {
  menu.SignalAssign().clear();
  menu.SignalAssign().connect([&]() { ShowAssignControllableDialog(); });
}

void ColorControlWidget::UpdateDisplaySettings() {
  name_label_.set_visible(State().DisplayName());
}

void ColorControlWidget::ShowAssignControllableDialog() {
  dialog_ = std::make_unique<ControllableSelectionDialog>(
      "Select item for color control", false);
  ControllableSelectionDialog &dialog =
      static_cast<ControllableSelectionDialog &>(*dialog_);
  dialog.SetFilter(ObjectListType::All);
  dialog.signal_response().connect([this](int response) {
    if (response == Gtk::ResponseType::OK) {
      const ControllableSelectionDialog &csd =
          static_cast<ControllableSelectionDialog &>(*dialog_);
      theatre::Controllable *controllable =
          dynamic_cast<theatre::Controllable *>(csd.SelectedObject().Get());
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
  });
  dialog.show();
}

}  // namespace glight::gui
