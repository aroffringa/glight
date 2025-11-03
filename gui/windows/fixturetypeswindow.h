#ifndef GUI_FIXTURE_TYPES_WINDOW_H_
#define GUI_FIXTURE_TYPES_WINDOW_H_

#include "fixturetypefunctionsframe.h"

#include "theatre/forwards.h"

#include "gui/recursionlock.h"
#include "gui/scopedconnection.h"
#include "gui/windows/childwindow.h"

#include <gtkmm/box.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/paned.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

#include <memory>

class FixtureSelection;

namespace glight::gui::windows {

/**
 * @author Andre Offringa
 */
class FixtureTypesWindow : public ChildWindow {
 public:
  FixtureTypesWindow();

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
  std::pair<theatre::FixtureType *, theatre::FixtureMode *> GetSelected();
  void Select(const theatre::FixtureMode &selection);
  void Select(const theatre::FixtureType &selection);
  void SelectFixtures(const theatre::FixtureMode &mode);
  void SelectFixtures(const theatre::FixtureType &type);
  void ShowTypeWidgets(bool visible);

  ScopedConnection update_controllables_connection_;
  RecursionLock recursion_lock_;

  Gtk::TreeView tree_view_;
  Glib::RefPtr<Gtk::TreeStore> tree_model_;
  struct TypesListColumns : public Gtk::TreeModelColumnRecord {
    TypesListColumns() {
      add(name_);
      add(functions_);
      add(in_use_);
      add(fixture_type_);
      add(fixture_mode_);
    }

    Gtk::TreeModelColumn<Glib::ustring> name_;
    Gtk::TreeModelColumn<Glib::ustring> functions_;
    Gtk::TreeModelColumn<bool> in_use_;
    Gtk::TreeModelColumn<theatre::FixtureType *> fixture_type_;
    Gtk::TreeModelColumn<theatre::FixtureMode *> fixture_mode_;
  } list_columns_;
  Gtk::ScrolledWindow type_scrollbars_;

  Gtk::Grid main_grid_;
  Gtk::Paned paned_;

  Gtk::Box left_box_{Gtk::Orientation::VERTICAL};

  Gtk::Grid right_grid_;
  Gtk::Label name_label_{"Name:"};
  Gtk::Entry name_entry_;
  Gtk::Label short_name_label_{"Short name:"};
  Gtk::Entry short_name_entry_;
  Gtk::Label class_label_{"Class:"};
  Gtk::ComboBoxText class_combo_;

  Gtk::Label beam_angle_label_{"Beam angle range:"};
  Gtk::Entry min_beam_angle_entry_;
  Gtk::Entry max_beam_angle_entry_;

  Gtk::Label pan_label_{"Pan range:"};
  Gtk::Entry min_pan_entry_;
  Gtk::Entry max_pan_entry_;

  Gtk::Label tilt_label_{"Tilt range:"};
  Gtk::Entry min_beam_tilt_entry_;
  Gtk::Entry max_beam_tilt_entry_;

  Gtk::Label brightness_label_{"Brightness:"};
  Gtk::Entry brightness_entry_;

  Gtk::Label max_power_label_{"Max power drawn:"};
  Gtk::Entry max_power_entry_;

  Gtk::Label idle_power_label_{"Idle power drawn:"};
  Gtk::Entry idle_power_entry_;

  FixtureTypeFunctionsFrame functions_frame_;

  // Bottom
  Gtk::Box button_box_;
  Gtk::Button new_button_{"New"};
  Gtk::Button remove_button_{"Remove"};
  Gtk::Button save_button_{"Save"};

  bool layout_locked_ = false;
};

}  // namespace glight::gui::windows

#endif
