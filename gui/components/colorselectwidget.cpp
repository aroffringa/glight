#include "colorselectwidget.h"

#include "gui/instance.h"

#include "gui/dialogs/controllableselectdialog.h"
#include "gui/dialogs/stringinputdialog.h"

#include "theatre/effects/variableeffect.h"
#include "theatre/management.h"

#include <gtkmm/colorchooserdialog.h>

namespace glight::gui {

ColorSelectWidget::ColorSelectWidget(Gtk::Window *parent, bool allow_variable)
    : parent_(parent),
      static_button_("Static"),
      variable_button_("Variable"),
      color_label_("Color"),
      set_button_("Set...") {
  Gtk::RadioButton::Group group;
  static_button_.set_group(group);
  static_button_.signal_clicked().connect([&]() {
    if (variable_label_.get_parent()) {
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

  pack_end(set_button_);
  set_button_.show();

  area_.signal_draw().connect(
      sigc::mem_fun(*this, &ColorSelectWidget::OnColorAreaDraw));
  pack_end(area_, true, true, 5);
  area_.show();
}

void ColorSelectWidget::OpenColorSelection() {
  Gdk::RGBA color;
  color.set_red(red_);
  color.set_green(green_);
  color.set_blue(blue_);
  color.set_alpha(1.0);  // opaque

  Gtk::ColorChooserDialog dialog("Select color for fixture");
  dialog.set_transient_for(*parent_);
  dialog.set_rgba(color);
  dialog.set_use_alpha(false);
  const std::vector<theatre::Color> color_set = theatre::Color::DefaultSet32();
  std::vector<Gdk::RGBA> colors;
  colors.reserve(color_set.size());
  for (const theatre::Color &color : color_set) {
    colors.emplace_back(color.RedRatio(), color.GreenRatio(),
                        color.BlueRatio());
  }
  dialog.add_palette(Gtk::Orientation::ORIENTATION_HORIZONTAL, 10, colors);
  const int result = dialog.run();

  // Handle the response:
  if (result == Gtk::RESPONSE_OK) {
    // Store the chosen color:
    color = dialog.get_rgba();
    red_ = color.get_red();
    green_ = color.get_green();
    blue_ = color.get_blue();
    signal_color_changed_();
    area_.queue_draw();
  }
}

void ColorSelectWidget::OpenVariableSelection() {
  ControllableSelectDialog dialog("Select color variable", true);
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
      theatre::Management &management = Instance::Get().Management();
      theatre::Effect &added = management.AddEffect(std::move(effect), parent);
      for (size_t i = 0; i != added.NInputs(); ++i)
        management.AddSourceValue(added, i);
      Instance::Get().Events().EmitUpdate();
      dialog.SelectObject(added);
    }
  });
  if (dialog.run() == Gtk::RESPONSE_OK) {
    theatre::VariableEffect *v =
        dynamic_cast<theatre::VariableEffect *>(dialog.SelectedObject());
    if (v) {
      variable_ = v;
      SetVariableLabel();
      signal_color_changed_();
    }
  }
}

void ColorSelectWidget::SetVariableLabel() {
  variable_label_.set_text(variable_->Name());
}

bool ColorSelectWidget::OnColorAreaDraw(
    const Cairo::RefPtr<Cairo::Context> &cr) const {
  cr->set_source_rgb(red_, green_, blue_);
  cr->paint();
  return true;
}

}  // namespace glight::gui
