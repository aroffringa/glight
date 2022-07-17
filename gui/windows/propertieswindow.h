#ifndef PROPERTIES_WINDOW_H
#define PROPERTIES_WINDOW_H

#include <gtkmm/window.h>

#include "../../theatre/forwards.h"

namespace glight::gui {

class PropertiesWindow : public Gtk::Window {
 public:
  virtual theatre::FolderObject &GetObject() = 0;
};

}  // namespace glight::gui

#endif
