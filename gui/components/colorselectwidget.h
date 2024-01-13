#ifndef COLOR_SELECT_WIDGET_H
#define COLOR_SELECT_WIDGET_H

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
    return theatre::Color(unsigned(red_ * 255.4), unsigned(green_ * 255.4),
                          unsigned(blue_ * 255.4));
  }
  void SetColor(const theatre::Color &color) {
    static_button_.set_active(true);
    red_ = double(color.Red()) / 255.0;
    green_ = double(color.Green()) / 255.0;
    blue_ = double(color.Blue()) / 255.0;
    signal_color_changed_();
    area_.queue_draw();
  }
  void SetVariable(theatre::VariableEffect *variable) {
    variable_button_.set_active(true);
    variable_ = variable;
    SetVariableLabel();
  }
  sigc::signal<void()> &SignalColorChanged() { return signal_color_changed_; }
  void SetAllowVariables(bool allow_variables) {
    if (allow_variables != allow_variables_) {
      allow_variables_ = allow_variables;
      if (allow_variables) {
        remove(color_label_);
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
        pack_end(color_label_, true, true, 0);
        color_label_.show();
      }
      area_.set_visible(true);
      signal_color_changed_();
    }
  }

 private:
  Gtk::Window *parent_;
  Gtk::RadioButton static_button_;
  Gtk::RadioButton variable_button_;
  Gtk::Label color_label_;
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
  void OnSetClicked();
  bool OnColorAreaDraw(const Cairo::RefPtr<Cairo::Context> &cr) const;

  void OpenColorSelection();
  void OpenVariableSelection();
};

}  // namespace glight::gui

#endif
