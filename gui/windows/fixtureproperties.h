#ifndef GUI_WINDOWS_FIXTURE_PROPERTIES_H_
#define GUI_WINDOWS_FIXTURE_PROPERTIES_H_

#include "../../theatre/forwards.h"

#include "../recursionlock.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/window.h>

#include <memory>

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
  FixtureProperties(EventTransmitter& event_hub,
                    theatre::Management& management,
                    FixtureSelection& fixture_selection);
  ~FixtureProperties();

 private:
  void update();
  void onSetClicked();

  EventTransmitter& event_hub_;
  theatre::Management& management_;
  FixtureSelection& selection_;

  sigc::connection update_controllables_connection_;
  sigc::connection selection_change_connection_;
  RecursionLock recursion_lock_;

  Gtk::Grid main_grid_;

  Gtk::Label direction_label_;
  Gtk::Entry direction_entry_;
  Gtk::Label tilt_label_;
  Gtk::Entry tilt_entry_;
  Gtk::Box button_box_;
  Gtk::Button set_button_;
};

}  // namespace glight::gui::windows

#endif
