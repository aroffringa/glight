#ifndef GLIGHT_GUI_WINDOWS_THEATRE_DIMENSIONS_H_
#define GLIGHT_GUI_WINDOWS_THEATRE_DIMENSIONS_H_

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>

#include "gui/recursionlock.h"
#include "gui/windows/childwindow.h"

namespace glight::gui::windows {

class TheatreDimensions : public ChildWindow {
 public:
  TheatreDimensions();

 private:
  void FillValues();
  void StoreValues();

  Gtk::Grid grid_;
  Gtk::Label width_label_{"Width:"};
  Gtk::Entry width_entry_;
  Gtk::Label depth_label_{"Depth:"};
  Gtk::Entry depth_entry_;
  Gtk::Label height_label_{"Height:"};
  Gtk::Entry height_entry_;
  Gtk::Label fixture_size_label_{"Fixture symbol size:"};
  Gtk::Entry fixture_size_entry_;
  Gtk::Box button_box_;
  Gtk::Button set_button_{"Set"};
};

}  // namespace glight::gui::windows

#endif
