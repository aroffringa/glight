#include "colorselectwidget.h"

#include "gui/instance.h"

#include "gui/tools.h"
#include "gui/dialogs/controllableselectiondialog.h"
#include "gui/dialogs/stringinputdialog.h"

#include "theatre/effects/variableeffect.h"
#include "theatre/management.h"

namespace glight::gui {

ColorSelectWidget::ColorSelectWidget(Gtk::Window *parent, bool allow_variable)
    : parent_(parent),
      static_button_("Static"),
      variable_button_("Variable"),
      set_button_("Set...") {
  Gtk::RadioButton::Group group;
  static_button_.set_group(group);
  static_button_.signal_clicked().connect([&]() {
    if (variable_label_.get_parent()) {
      remove(set_button_);
      remove(variable_label_);
      pack_end(area_, true, true, 5);
      area_.show();
      signal_color_changed_();
    }
  });
  pack_start(static_button_, false, false, 0);
  static_button_.show();

  variable_button_.set_group(group);
  variable_button_.signal_clicked().connect([&]() {
    if (area_.get_parent()) {
      remove(area_);
      pack_end(variable_label_);
      variable_label_.show();
      pack_end(set_button_);
      set_button_.show();
      signal_color_changed_();
    }
  });
  pack_start(variable_button_, true, true, 0);
  variable_button_.show();

  set_button_.signal_clicked().connect([&]() {
    if (static_button_.get_active())
      OpenColorSelection();
    else
      OpenVariableSelection();
  });

  area_.set_size_request(35, 35);
  area_.signal_draw().connect(
      sigc::mem_fun(*this, &ColorSelectWidget::OnColorAreaDraw));
  pack_end(area_, true, true, 5);
  area_.set_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
  area_.signal_button_release_event().connect([&](GdkEventButton *) {
    OpenColorSelection();
    return false;
  });
  area_.show();

  SetAllowVariables(allow_variable);
}

void ColorSelectWidget::OpenColorSelection() {
  const std::optional<theatre::Color> color =
      OpenColorDialog(*parent_, theatre::Color::FromRatio(red_, green_, blue_));
  if (color) {
    red_ = color->RedRatio();
    green_ = color->GreenRatio();
    blue_ = color->BlueRatio();
    signal_color_changed_();
    area_.queue_draw();
  }
}

void ColorSelectWidget::OpenVariableSelection() {
  ControllableSelectionDialog dialog("Select color variable", true);
  dialog.SetFilter(ObjectListType::OnlyVariables);
  dialog.ShowNewButton(true);
  dialog.SignalNewClicked().connect([&]() {
    StringInputDialog string_dialog("New variable",
                                    "Name of new variable:", "");
    if (string_dialog.run() == Gtk::RESPONSE_OK) {
      std::unique_ptr<theatre::Effect> effect =
          std::make_unique<theatre::VariableEffect>();
      theatre::Folder &parent = dialog.SelectedFolder();
      effect->SetName(string_dialog.Value());
      theatre::Management &management = Instance::Management();
      theatre::Effect &added = management.AddEffect(std::move(effect), parent);
      for (size_t i = 0; i != added.NInputs(); ++i)
        management.AddSourceValue(added, i);
      Instance::Events().EmitUpdate();
      dialog.SelectObject(added);
    }
  });
  if (dialog.run() == Gtk::RESPONSE_OK) {
    theatre::VariableEffect *v =
        dynamic_cast<theatre::VariableEffect *>(dialog.SelectedObject());
    if (v && v != variable_) {
      variable_ = v;
      SetVariableLabel();
      signal_color_changed_();
    }
  }
}

void ColorSelectWidget::SetVariableLabel() {
  if (variable_)
    variable_label_.set_text(variable_->Name());
  else
    variable_label_.set_text("");
}

bool ColorSelectWidget::OnColorAreaDraw(
    const Cairo::RefPtr<Cairo::Context> &cr) const {
  constexpr int border = 7;
  const int width = area_.get_width() - 2 * border;
  const int height = area_.get_height() - 2 * border;
  cr->rectangle(border, border, width, height);
  cr->set_source_rgb(red_, green_, blue_);
  cr->fill();
  cr->rectangle(border + 1, border + 1, width - 2, height - 2);
  cr->set_source_rgb(0.75, 0.75, 0.75);
  cr->stroke();
  cr->rectangle(border, border, width, height);
  cr->set_source_rgb(0.5, 0.5, 0.5);
  cr->stroke();
  return true;
}

}  // namespace glight::gui
