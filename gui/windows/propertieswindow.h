#ifndef PROPERTIES_WINDOW_H
#define PROPERTIES_WINDOW_H

#include <gtkmm/window.h>

class PropertiesWindow : public Gtk::Window {
 public:
  virtual class FolderObject &GetObject() = 0;
};

#endif
