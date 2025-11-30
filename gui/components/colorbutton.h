#ifndef GLIGHT_GUI_COMPONENTS_DISPLAY_COLOR_H_
#define GLIGHT_GUI_COMPONENTS_DISPLAY_COLOR_H_

#include <gtkmm/drawingarea.h>
#include <gtkmm/gestureclick.h>

#include "theatre/color.h"

namespace glight::gui::components {

class ColorButton : public Gtk::DrawingArea {
 public:
  ColorButton(const theatre::Color& color = theatre::Color::White())
      : color_(color) {
    set_draw_func([&](const Cairo::RefPtr<Cairo::Context>& cairo, int, int) {
      DrawColor(cairo);
    });

    auto gesture = Gtk::GestureClick::create();
    gesture->set_button(1);
    gesture->signal_released().connect(
        [this](int, double, double) { signal_clicked_(); });
    add_controller(gesture);
  }

  void SetColor(const theatre::Color& color) {
    color_ = color;
    queue_draw();
  }

  theatre::Color GetColor() const { return color_; }

  sigc::signal<void()>& SignalClicked() { return signal_clicked_; }

 private:
  void DrawColor(const Cairo::RefPtr<Cairo::Context>& cr) const {
    constexpr int border = 7;
    const int width = std::max(0, get_width() - 2 * border);
    const int height = std::max(0, get_height() - 2 * border);
    cr->rectangle(border, border, width, height);
    const double red = color_.RedRatio();
    const double green = color_.GreenRatio();
    const double blue = color_.BlueRatio();
    cr->set_source_rgb(red, green, blue);
    cr->fill();
    cr->rectangle(border + 1, border + 1, width - 2, height - 2);
    cr->set_source_rgb(0.75, 0.75, 0.75);
    cr->stroke();
    cr->rectangle(border, border, width, height);
    cr->set_source_rgb(0.5, 0.5, 0.5);
    cr->stroke();
  }

  theatre::Color color_;
  sigc::signal<void()> signal_clicked_;
};

}  // namespace glight::gui::components

#endif
