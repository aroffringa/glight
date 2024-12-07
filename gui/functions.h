#ifndef GLIGHT_GUI_TOOLS_H_
#define GLIGHT_GUI_TOOLS_H_

#include <optional>

#include "theatre/color.h"

#include <gtkmm/window.h>

namespace glight::theatre {
class Controllable;
}

namespace glight::gui {

std::optional<theatre::Color> OpenColorDialog(
    Gtk::Window& parent, const theatre::Color& start_color);

void AssignFader(theatre::Controllable& controllable);

}  // namespace glight::gui

#endif
