#include "colorselectwidget.h"

#include "gui/instance.h"

#include "gui/functions.h"
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
  static_button_.signal_toggled().connect([&]() {
    if (variable_label_.get_parent()) {
      remove(set_button_);
      remove(variable_label_);
      append(color_button_);
      color_button_.show();
      signal_color_changed_();
    }
  });
  append(static_button_);
  static_button_.show();

  variable_button_.set_group(variable_button_);
  variable_button_.signal_toggled().connect([&]() {
    if (color_button_.get_parent()) {
      remove(color_button_);
      append(variable_label_);
      variable_label_.show();
      append(set_button_);
      set_button_.show();
      signal_color_changed_();
    }
  });
  append(variable_button_);
  variable_button_.show();

  set_button_.signal_clicked().connect([&]() {
    if (static_button_.get_active())
      OpenColorSelection();
    else
      OpenVariableSelection();
  });

  color_button_.set_size_request(35, 35);
  append(color_button_);
  color_button_.SignalClicked().connect([&]() { OpenColorSelection(); });
  color_button_.show();

  SetAllowVariables(allow_variable);
}

void ColorSelectWidget::OpenColorSelection() {
  OpenColorDialog(dialog_, *parent_, color_button_.GetColor(),
                  [this](theatre::Color color) {
                    color_button_.SetColor(color);
                    signal_color_changed_();
                  });
}

void ColorSelectWidget::OpenVariableSelection() {
  ControllableSelectionDialog dialog("Select color variable", true);
  dialog.SetFilter(ObjectListType::OnlyVariables);
  dialog.ShowNewButton(true);
  dialog.SignalNewClicked().connect([&]() {
    StringInputDialog string_dialog("New variable",
                                    "Name of new variable:", "");
    string_dialog.signal_response().connect([&](int response) {
      if (response == Gtk::ResponseType::OK) {
        std::unique_ptr<theatre::Effect> effect =
            std::make_unique<theatre::VariableEffect>();
        theatre::Folder &parent = dialog.SelectedFolder();
        effect->SetName(string_dialog.Value());
        theatre::Management &management = Instance::Management();
        system::ObservingPtr<theatre::Effect> added =
            management.AddEffectPtr(std::move(effect), parent);
        for (size_t i = 0; i != added->NInputs(); ++i)
          management.AddSourceValue(*added, i);
        Instance::Events().EmitUpdate();
        dialog.SelectObject(*added);
      }
    });
    string_dialog.show();
  });
  dialog.signal_response().connect([&](int response) {
    theatre::VariableEffect *v =
        dynamic_cast<theatre::VariableEffect *>(dialog.SelectedObject().Get());
    if (v && v != variable_) {
      variable_ = v;
      SetVariableLabel();
      signal_color_changed_();
    }
  });
  dialog.show();
}

void ColorSelectWidget::SetVariableLabel() {
  if (variable_)
    variable_label_.set_text(variable_->Name());
  else
    variable_label_.set_text("");
}

}  // namespace glight::gui
