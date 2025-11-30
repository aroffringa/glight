#ifndef GUI_WINDOWS_FIXTURE_PROPERTIES_H_
#define GUI_WINDOWS_FIXTURE_PROPERTIES_H_

#include <memory>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/window.h>

#include "gui/recursionlock.h"
#include <sigc++/scoped_connection.h>

#include "theatre/forwards.h"

namespace glight::gui {
class EventTransmitter;
class FixtureSelection;
}  // namespace glight::gui

namespace glight::gui::windows {

/**
 * @author Andre Offringa
 */
class FixtureProperties : public Gtk::Window {
 public:
  FixtureProperties();

 private:
  void update();
  void onSetClicked();

  sigc::scoped_connection update_controllables_connection_;
  sigc::scoped_connection selection_change_connection_;
  RecursionLock recursion_lock_;

  Gtk::Grid main_grid_;

  Gtk::Label height_label_{"Height:"};
  Gtk::Entry height_entry_;
  Gtk::Label direction_label_{"Direction:"};
  Gtk::Entry direction_entry_;
  Gtk::Label static_tilt_label_{"Tilt:"};
  Gtk::Entry static_tilt_entry_;
  Gtk::CheckButton upside_down_cb_{"Is upside down"};
  Gtk::Label phase_label_{"Electric phase:"};
  Gtk::Entry phase_entry_;
  Gtk::Box button_box_;
  Gtk::Button set_button_{"Set"};
};

}  // namespace glight::gui::windows

#endif
