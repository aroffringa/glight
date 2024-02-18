#include "tools.h"

#include <gtkmm/colorchooserdialog.h>

namespace glight::gui {

std::optional<theatre::Color> OpenColorDialog(
    Gtk::Window& parent, const theatre::Color& start_color) {
  Gtk::ColorChooserDialog dialog("Select color for fixture");
  dialog.set_transient_for(parent);
  Gdk::RGBA rgba_color;
  rgba_color.set_red(start_color.RedRatio());
  rgba_color.set_green(start_color.GreenRatio());
  rgba_color.set_blue(start_color.BlueRatio());
  rgba_color.set_alpha(1.0);  // opaque
  dialog.set_rgba(rgba_color);
  dialog.set_use_alpha(false);
  const std::vector<theatre::Color> color_set = theatre::Color::DefaultSet40();
  std::vector<Gdk::RGBA> colors;
  colors.reserve(color_set.size());
  for (const theatre::Color& color : color_set) {
    colors.emplace_back(color.RedRatio(), color.GreenRatio(),
                        color.BlueRatio());
  }
  dialog.add_palette(Gtk::Orientation::ORIENTATION_HORIZONTAL, 10, colors);
  const int result = dialog.run();

  // Handle the response:
  if (result == Gtk::RESPONSE_OK) {
    rgba_color = dialog.get_rgba();
    return theatre::Color::FromRatio(
        rgba_color.get_red(), rgba_color.get_green(), rgba_color.get_blue());
  } else {
    return {};
  }
}

}  // namespace glight::gui
