#ifndef COLOR_SELECT_WIDGET_H
#define COLOR_SELECT_WIDGET_H

#include <optional>
#include <variant>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>

#include "theatre/color.h"
#include "theatre/forwards.h"

#include "gui/eventtransmitter.h"

namespace glight::theatre {
class VariableEffect;
}

namespace glight::gui {

class ColorSelectWidget : public Gtk::HBox {
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
  theatre::Color GetColor() const {
    return theatre::Color::FromRatio(red_, green_, blue_);
  }
  void SetColor(const theatre::Color &color) {
    bool is_changed = false;
    if (!static_button_.get_active()) {
      static_button_.set_active(true);
      is_changed = true;
    }
    const auto change = [](double &old_value, double new_value, bool &change) {
      if (old_value != new_value) {
        old_value = new_value;
        change = true;
      }
    };
    change(red_, double(color.Red()) / 255.0, is_changed);
    change(green_, double(color.Green()) / 255.0, is_changed);
    change(blue_, double(color.Blue()) / 255.0, is_changed);
    if (is_changed) {
      signal_color_changed_();
      area_.queue_draw();
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
        pack_start(static_button_, false, false, 0);
        static_button_.show();
        pack_start(variable_button_, true, true, 0);
        variable_button_.show();
        static_button_.set_active(true);
        variable_label_.set_visible(false);
      } else {
        static_button_.set_active(true);
        remove(static_button_);
        remove(variable_button_);
      }
      area_.set_visible(true);
      signal_color_changed_();
    }
  }

 private:
  Gtk::Window *parent_;
  Gtk::RadioButton static_button_;
  Gtk::RadioButton variable_button_;
  Gtk::DrawingArea area_;
  Gtk::Label variable_label_;
  Gtk::Button set_button_;
  bool allow_variables_ = true;
  double red_ = 1.0;
  double green_ = 1.0;
  double blue_ = 1.0;
  theatre::VariableEffect *variable_ = nullptr;
  sigc::signal<void()> signal_color_changed_;

  void SetVariableLabel();
  bool OnColorAreaDraw(const Cairo::RefPtr<Cairo::Context> &cr) const;

  void OpenColorSelection();
  void OpenVariableSelection();
};

}  // namespace glight::gui

#endif
