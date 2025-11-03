#ifndef COLOR_SELECT_WIDGET_H
#define COLOR_SELECT_WIDGET_H

#include <optional>
#include <variant>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/dialog.h>
#include <gtkmm/label.h>

#include "theatre/color.h"
#include "theatre/forwards.h"

#include "gui/eventtransmitter.h"
#include "gui/components/colorbutton.h"

namespace glight::theatre {
class VariableEffect;
}

namespace glight::gui {

class ColorSelectWidget : public Gtk::Box {
 public:
  ColorSelectWidget(Gtk::Window *parent, bool allow_variable);
  bool IsVariable() const { return variable_button_.get_active(); }
  void SetSelection(const theatre::ColorOrVariable &color_or_variable) {
    std::visit(
        [&](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, theatre::Color>)
            SetColor(arg);
          else if constexpr (std::is_same_v<T, theatre::VariableEffect *>)
            SetVariable(arg);
        },
        color_or_variable);
  }
  theatre::ColorOrVariable GetSelection() const {
    if (IsVariable())
      return {variable_};
    else
      return {GetColor()};
  }
  theatre::Color GetColor() const { return color_button_.GetColor(); }
  void SetColor(const theatre::Color &color) {
    bool is_changed = false;
    if (!static_button_.get_active()) {
      static_button_.set_active(true);
      is_changed = true;
    }
    if (color != color_button_.GetColor()) {
      color_button_.SetColor(color);
      is_changed = true;
    }
    if (is_changed) {
      signal_color_changed_();
    }
  }
  void SetVariable(theatre::VariableEffect *variable) {
    bool is_changed = false;
    if (!variable_button_.get_active()) {
      variable_button_.set_active(true);
      is_changed = true;
    }
    if (variable_ != variable) {
      variable_ = variable;
      is_changed = true;
    }
    if (is_changed) {
      signal_color_changed_();
      SetVariableLabel();
    }
  }
  sigc::signal<void()> &SignalColorChanged() { return signal_color_changed_; }
  void SetAllowVariables(bool allow_variables) {
    if (allow_variables != allow_variables_) {
      allow_variables_ = allow_variables;
      if (allow_variables) {
        append(static_button_);
        static_button_.show();
        append(variable_button_);
        variable_button_.show();
        static_button_.set_active(true);
        variable_label_.set_visible(false);
      } else {
        static_button_.set_active(true);
        remove(static_button_);
        remove(variable_button_);
      }
      color_button_.set_visible(true);
      signal_color_changed_();
    }
  }

 private:
  Gtk::Window *parent_;
  Gtk::CheckButton static_button_;
  Gtk::CheckButton variable_button_;
  components::ColorButton color_button_{theatre::Color::White()};
  Gtk::Label variable_label_;
  Gtk::Button set_button_;
  bool allow_variables_ = true;
  theatre::VariableEffect *variable_ = nullptr;
  sigc::signal<void()> signal_color_changed_;
  std::unique_ptr<Gtk::Dialog> dialog_;

  void SetVariableLabel();

  void OpenColorSelection();
  void OpenVariableSelection();
};

}  // namespace glight::gui

#endif
