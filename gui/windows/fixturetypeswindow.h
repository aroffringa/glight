#ifndef FIXTURE_TYPES_WINDOW_H
#define FIXTURE_TYPES_WINDOW_H

#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
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
  void onSaveButtonClicked();
  void onSelectionChanged();
  void onAddFunction();
  void onRemoveFunction();
  void onSelectedFunctionChanged();
  FixtureType *getSelected();
  std::vector<FixtureTypeFunction> getFunctions() const;

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

  Gtk::VBox left_box_;

  Gtk::Grid right_grid_;
  Gtk::Label name_label_;
  Gtk::Entry name_entry_;
  Gtk::Label class_label_;
  Gtk::ComboBoxText class_combo_;
  Gtk::Label functions_label_;
  Gtk::TreeView functions_view_;
  Glib::RefPtr<Gtk::ListStore> functions_model_;
  struct FunctionsColumns : public Gtk::TreeModelColumnRecord {
    FunctionsColumns() {
      add(dmx_offset_);
      add(is_16_bit_);
      add(function_type_);
      add(function_type_str_);
    }

    Gtk::TreeModelColumn<size_t> dmx_offset_;
    Gtk::TreeModelColumn<bool> is_16_bit_;
    Gtk::TreeModelColumn<FunctionType> function_type_;
    Gtk::TreeModelColumn<Glib::ustring> function_type_str_;
  } functions_columns_;
  Gtk::HBox functions_button_box_;
  Gtk::Button add_function_button_;
  Gtk::Button remove_function_button_;

  Gtk::HButtonBox button_box_;
  Gtk::Button new_button_;
  Gtk::Button remove_button_;
  Gtk::Button save_button_;
};

#endif
