#ifndef GLIGHT_GUI_TOOLS_H_
#define GLIGHT_GUI_TOOLS_H_

#include <memory>

#include "theatre/color.h"

#include <gtkmm/dialog.h>
#include <gtkmm/window.h>

namespace glight::theatre {
class Controllable;
}

namespace glight::gui {

void OpenColorDialog(std::unique_ptr<Gtk::Dialog>& dialog, Gtk::Window& parent,
                     const theatre::Color& start_color,
                     std::function<void(theatre::Color)> callback);

void AssignFader(theatre::Controllable& controllable);

}  // namespace glight::gui

#endif
