#ifndef COLOR_SELECT_WIDGET_H
#define COLOR_SELECT_WIDGET_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/label.h>

#include "../../theatre/color.h"

namespace glight::gui {

class ColorSelectWidget : public Gtk::HBox {
 public:
  ColorSelectWidget(Gtk::Window *parent);

  const theatre::Color GetColor() const {
    return theatre::Color(unsigned(_colorR * 255.4), unsigned(_colorG * 255.4),
                          unsigned(_colorB * 255.4));
  }
  void SetColor(const theatre::Color &color) {
    _colorR = double(color.Red()) / 255.0;
    _colorG = double(color.Green()) / 255.0;
    _colorB = double(color.Blue()) / 255.0;
    _signalColorChanged();
    _area.queue_draw();
  }
  sigc::signal<void()> &SignalColorChanged() { return _signalColorChanged; }

 private:
  Gtk::Window *_parent;
  Gtk::Label _label;
  Gtk::DrawingArea _area;
  Gtk::Button _button;
  double _colorR, _colorG, _colorB;
  sigc::signal<void()> _signalColorChanged;

  void onColorClicked();
  bool onColorAreaDraw(const Cairo::RefPtr<Cairo::Context> &cr) const;
};

}  // namespace glight::gui

#endif
