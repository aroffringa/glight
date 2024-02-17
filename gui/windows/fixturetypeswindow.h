#ifndef GUI_FIXTURE_TYPES_WINDOW_H_
#define GUI_FIXTURE_TYPES_WINDOW_H_

#include "fixturetypefunctionsframe.h"

#include "../../theatre/forwards.h"

#include "../recursionlock.h"

#include <gtkmm/box.h>
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

  void SetLayoutLocked(bool locked) {
    layout_locked_ = locked;
    new_button_.set_sensitive(!locked);
    onSelectionChanged();
  }

 private:
  void update() { fillList(); }
  void fillList();
  void onNewButtonClicked();
  void onRemoveClicked();
  void onSaveClicked();
  void onSelectionChanged();
  theatre::FixtureType *getSelected();
  void Select(const theatre::FixtureType &selection);

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
  Gtk::ScrolledWindow type_scrollbars_;

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

  Gtk::Label min_beam_angle_label_;
  Gtk::Entry min_beam_angle_entry_;
  Gtk::Label max_beam_angle_label_;
  Gtk::Entry max_beam_angle_entry_;

  Gtk::Label min_pan_label_;
  Gtk::Entry min_pan_entry_;
  Gtk::Label max_pan_label_;
  Gtk::Entry max_pan_entry_;

  Gtk::Label min_tilt_label_;
  Gtk::Entry min_beam_tilt_entry_;
  Gtk::Label max_tilt_label_;
  Gtk::Entry max_beam_tilt_entry_;

  Gtk::Label brightness_label_;
  Gtk::Entry brightness_entry_;

  FixtureTypeFunctionsFrame functions_frame_;

  Gtk::Box button_box_;
  Gtk::Button new_button_;
  Gtk::Button remove_button_;
  Gtk::Button save_button_;

  bool layout_locked_ = false;
};

}  // namespace glight::gui

#endif
