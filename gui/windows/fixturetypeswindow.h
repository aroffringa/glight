#ifndef FIXTURE_TYPES_WINDOW_H
#define FIXTURE_TYPES_WINDOW_H

#include "fixturetypefunctionsframe.h"

#include "../../theatre/fixturetype.h"

#include "../recursionlock.h"

#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/liststore.h>
#include <gtkmm/menu.h>
#include <gtkmm/paned.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

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
  void onRemoveClicked();
  void onSaveClicked();
  void onSelectionChanged();
  FixtureType *getSelected();

  EventTransmitter *event_hub_;
  Management *management_;

  sigc::connection change_management_connection_;
  sigc::connection update_controllables_connection_;
  RecursionLock recursion_lock_;

  Gtk::TreeView list_view_;
  Glib::RefPtr<Gtk::ListStore> list_model_;
  struct TypesListColumns : public Gtk::TreeModelColumnRecord {
    TypesListColumns() {
      add(name_);
      add(functions_);
      add(in_use_);
      add(fixture_type_);
    }

    Gtk::TreeModelColumn<Glib::ustring> name_;
    Gtk::TreeModelColumn<Glib::ustring> functions_;
    Gtk::TreeModelColumn<bool> in_use_;
    Gtk::TreeModelColumn<FixtureType *> fixture_type_;
  } list_columns_;
  Gtk::ScrolledWindow scrolled_window_;

  Gtk::Grid main_grid_;
  Gtk::Paned paned_;

  Gtk::VBox left_box_;

  Gtk::Grid right_grid_;
  Gtk::Label name_label_;
  Gtk::Entry name_entry_;
  Gtk::Label class_label_;
  Gtk::ComboBoxText class_combo_;

  FixtureTypeFunctionsFrame functions_frame_;

  Gtk::HButtonBox button_box_;
  Gtk::Button new_button_;
  Gtk::Button remove_button_;
  Gtk::Button save_button_;
};

#endif
