#ifndef GUI_FIXTURE_TYPES_WINDOW_H_
#define GUI_FIXTURE_TYPES_WINDOW_H_

#include "fixturetypefunctionsframe.h"

#include "../../theatre/forwards.h"

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

namespace glight::gui {

class EventTransmitter;
class FixtureSelection;

/**
 * @author Andre Offringa
 */
class FixtureTypesWindow : public Gtk::Window {
 public:
  FixtureTypesWindow(EventTransmitter *eventHub,
                     theatre::Management &management);
  ~FixtureTypesWindow();

 private:
  void update() { fillList(); }
  void fillList();
  void onNewButtonClicked();
  void onRemoveClicked();
  void onSaveClicked();
  void onSelectionChanged();
  theatre::FixtureType *getSelected();

  EventTransmitter *event_hub_;
  theatre::Management *management_;

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
    Gtk::TreeModelColumn<theatre::FixtureType *> fixture_type_;
  } list_columns_;
  Gtk::ScrolledWindow scrolled_window_;

  Gtk::Grid main_grid_;
  Gtk::Paned paned_;

  Gtk::VBox left_box_;

  Gtk::Grid right_grid_;
  Gtk::Label name_label_;
  Gtk::Entry name_entry_;
  Gtk::Label short_name_label_;
  Gtk::Entry short_name_entry_;
  Gtk::Label class_label_;
  Gtk::ComboBoxText class_combo_;
  Gtk::Label beam_angle_label_;
  Gtk::Entry beam_angle_entry_;
  Gtk::Label brightness_label_;
  Gtk::Entry brightness_entry_;

  FixtureTypeFunctionsFrame functions_frame_;

  Gtk::HButtonBox button_box_;
  Gtk::Button new_button_;
  Gtk::Button remove_button_;
  Gtk::Button save_button_;
};

}  // namespace glight::gui

#endif
