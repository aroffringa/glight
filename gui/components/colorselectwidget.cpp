#include "colorselectwidget.h"

#include <gtkmm/colorchooserdialog.h>

namespace glight::gui {

ColorSelectWidget::ColorSelectWidget(Gtk::Window *parent)
    : _parent(parent),
      _label("Color:"),
      _button("Change..."),
      _colorR(1.0),
      _colorG(1.0),
      _colorB(1.0) {
  pack_start(_label);
  _button.signal_clicked().connect(
      sigc::mem_fun(*this, &ColorSelectWidget::onColorClicked));
  pack_end(_button);
  _area.signal_draw().connect(
      sigc::mem_fun(*this, &ColorSelectWidget::onColorAreaDraw));
  pack_end(_area, true, true, 5);
  show_all_children();
}

void ColorSelectWidget::onColorClicked() {
  Gdk::RGBA color;
  color.set_red(_colorR);
  color.set_green(_colorG);
  color.set_blue(_colorB);
  color.set_alpha(1.0);  // opaque

  Gtk::ColorChooserDialog dialog("Select color for fixture");
  dialog.set_transient_for(*_parent);
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
      Gdk::RGBA("#E0FF80"), Gdk::RGBA("#FFFF80"), Gdk::RGBA("#FFFFC0"),
      Gdk::RGBA("#FFC0C0"),
      Gdk::RGBA("#FFD4D4"),  // warm white
      Gdk::RGBA("#FFE2E2"), Gdk::RGBA("#FFF0F0"),
      Gdk::RGBA("#FFFFFF"),  // white
      Gdk::RGBA("#F0F0FF"), Gdk::RGBA("#E2E2FF"),
      Gdk::RGBA("#D4D4FF"),  // cold white
  };
  dialog.add_palette(Gtk::Orientation::ORIENTATION_HORIZONTAL, 10, colors);
  const int result = dialog.run();

  // Handle the response:
  if (result == Gtk::RESPONSE_OK) {
    // Store the chosen color:
    color = dialog.get_rgba();
    _colorR = color.get_red();
    _colorG = color.get_green();
    _colorB = color.get_blue();
    _signalColorChanged();
    _area.queue_draw();
  }
}

bool ColorSelectWidget::onColorAreaDraw(
    const Cairo::RefPtr<Cairo::Context> &cr) {
  cr->set_source_rgb(_colorR, _colorG, _colorB);
  cr->paint();
  return true;
}

}  // namespace glight::gui
