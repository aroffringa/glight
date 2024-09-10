#ifndef GLIGHT_GUI_WINDOWS_CHILD_WINDOW_H_
#define GLIGHT_GUI_WINDOWS_CHILD_WINDOW_H_

#include <gtkmm/window.h>

namespace glight::gui::windows {

class ChildWindow : public Gtk::Window {
 public:
  virtual void SetLayoutLocked(bool layout_is_locked) {}
};

}  // namespace glight::gui::windows

#endif
