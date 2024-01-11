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
      set_button_("Set...") {
  Gtk::RadioButton::Group group;
  static_button_.set_group(group);
  static_button_.signal_clicked().connect([&]() {
    variable_label_.set_visible(false);
    area_.set_visible(true);
    signal_color_changed_();
  });
  pack_start(static_button_, false, false, 0);
  static_button_.show();

  variable_button_.set_group(group);
  variable_button_.signal_clicked().connect([&]() {
    variable_label_.set_visible(true);
    area_.set_visible(false);
    signal_color_changed_();
  });
  pack_start(variable_button_, true, true, 0);
  variable_button_.show();

  set_button_.signal_clicked().connect([&]() {
    if (static_button_.get_active())
      OpenColorSelection();
    else
      OpenVariableSelection();
  });
  set_button_.show();

  pack_end(set_button_);
  set_button_.show();

  pack_end(variable_label_);
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
  const std::vector<Gdk::RGBA> colors{
      Gdk::RGBA("#FF0000"), Gdk::RGBA("#FF4000"), Gdk::RGBA("#FF8000"),
      Gdk::RGBA("#FFC000"), Gdk::RGBA("#FFFF00"), Gdk::RGBA("#C0FF00"),
      Gdk::RGBA("#80FF00"), Gdk::RGBA("#00FF00"), Gdk::RGBA("#00FF80"),
      Gdk::RGBA("#00FFC0"),  // 10
      Gdk::RGBA("#FF0080"), Gdk::RGBA("#FF00FF"), Gdk::RGBA("#C000FF"),
      Gdk::RGBA("#8000FF"), Gdk::RGBA("#4000FF"), Gdk::RGBA("#0000FF"),
      Gdk::RGBA("#0040FF"), Gdk::RGBA("#0080FF"), Gdk::RGBA("#00C0FF"),
      Gdk::RGBA("#00FFFF"),  // 20
      Gdk::RGBA("#FF8080"), Gdk::RGBA("#FF80FF"), Gdk::RGBA("#C080FF"),
      Gdk::RGBA("#8080FF"), Gdk::RGBA("#80C0FF"), Gdk::RGBA("#80FFFF"),
      Gdk::RGBA("#80FFC0"), Gdk::RGBA("#80FF80"), Gdk::RGBA("#C0FF80"),
      Gdk::RGBA("#E0FF80"),  // 30
      Gdk::RGBA("#FFFF80"), Gdk::RGBA("#FFFFC0"), Gdk::RGBA("#FFC0C0"),
      Gdk::RGBA("#FFAA5F"),  // warm white (2700 K)
      Gdk::RGBA("#FFBA81"),  // 3200 K
      Gdk::RGBA("#FFD0AA"),  // 4000 K
      Gdk::RGBA("#FFE2CA"),  // 4800 K
      Gdk::RGBA("#FFFFFF"),  // sRGB neutral white (6500 K)
      Gdk::RGBA("#D5E1FF"),  // 8500 K
      Gdk::RGBA("#C5D7FF"),  // 10500 K
  };
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
