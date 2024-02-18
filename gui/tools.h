#ifndef GLIGHT_GUI_TOOLS_H_
#define GLIGHT_GUI_TOOLS_H_

#include <optional>

#include <gtkmm/window.h>

#include "theatre/color.h"

namespace glight::gui {

std::optional<theatre::Color> OpenColorDialog(
    Gtk::Window& parent, const theatre::Color& start_color);

}

#endif
