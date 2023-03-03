#ifndef GUI_COMPONENTS_FIXTURE_LIST_H_
#define GUI_COMPONENTS_FIXTURE_LIST_H_

#include <vector>

#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>

#include "../../theatre/forwards.h"

namespace glight::gui {
class EventTransmitter;
}

namespace glight::gui::components {

class FixtureList : public Gtk::ScrolledWindow {
 public:
  FixtureList(theatre::Management &management, EventTransmitter &hub);

  std::vector<theatre::Fixture *> Selection() const;
  void Select(const std::vector<theatre::Fixture *> &fixtures);

 private:
  void Fill();

  Gtk::TreeView view_;
  Glib::RefPtr<Gtk::ListStore> model_;
  struct Columns : public Gtk::TreeModelColumnRecord {
    Columns() {
      add(title_);
      add(type_);
      add(fixture_);
      add(group_);
    }

    Gtk::TreeModelColumn<Glib::ustring> title_;
    Gtk::TreeModelColumn<Glib::ustring> type_;
    Gtk::TreeModelColumn<theatre::Fixture *> fixture_;
    Gtk::TreeModelColumn<theatre::FixtureGroup *> group_;
  } columns_;
  theatre::Management &management_;
  EventTransmitter &hub_;
};

}  // namespace glight::gui::components

#endif
