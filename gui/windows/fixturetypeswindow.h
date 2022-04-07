#ifndef FIXTURE_TYPES_WINDOW_H
#define FIXTURE_TYPES_WINDOW_H

#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/menu.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

#include "../../theatre/fixturetype.h"

#include "../recursionlock.h"

#include <memory>

class EventTransmitter;
class Fixture;
class FixtureSelection;
class Management;

/**
 * @author Andre Offringa
 */
class FixtureTypesWindow : public Gtk::Window {
 public:
  FixtureTypesWindow(EventTransmitter *eventHub, Management &management);
  ~FixtureTypesWindow();

 private:
  void onChangeManagement(Management &management) {
    management_ = &management;
    fillList();
  }
  void update() { fillList(); }
  void fillList();
  void onNewButtonClicked();
  void onRemoveButtonClicked();
  void onSelectionChanged();
  FixtureType *getSelected();

  EventTransmitter *event_hub_;
  Management *management_;

  sigc::connection change_management_connection_;
  sigc::connection update_controllables_connection_;
  RecursionLock recursion_lock_;

  Gtk::TreeView list_view_;
  Glib::RefPtr<Gtk::ListStore> list_model_;
  struct FixtureTypesListColumns : public Gtk::TreeModelColumnRecord {
    FixtureTypesListColumns() {
      add(name_);
      add(functions_);
      add(fixture_type_);
    }

    Gtk::TreeModelColumn<Glib::ustring> name_, functions_;
    Gtk::TreeModelColumn<FixtureType *> fixture_type_;
  } list_columns_;
  Gtk::ScrolledWindow scrolled_window_;

  Gtk::VBox main_box_;
  Gtk::HButtonBox button_box_;

  Gtk::Button new_button_, remove_button_;
};

#endif
