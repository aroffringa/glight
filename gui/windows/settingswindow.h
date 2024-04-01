#ifndef GUI_SETTINGS_WINDOW_H_
#define GUI_SETTINGS_WINDOW_H_

#include <gtkmm/box.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/frame.h>
#include <gtkmm/grid.h>
#include <gtkmm/liststore.h>
#include <gtkmm/notebook.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

#include "gui/recursionlock.h"

#include "theatre/devices/universemap.h"

namespace glight::gui {

class SettingsWindow : public Gtk::Window {
 public:
  SettingsWindow();

  void SetPointer(std::unique_ptr<SettingsWindow> self) {
    self_ = std::move(self);
  }

 private:
  void FillUniverses();
  void SetUniverseRow(const theatre::devices::UniverseMap& universes,
                      size_t universe, Gtk::TreeRow& row);
  void UpdateAfterSelection();
  void SaveSelectedUniverse();
  void SaveSelectedOlaUniverse();
  void ReloadOla();

  std::unique_ptr<SettingsWindow> self_;
  Gtk::Notebook notebook_;
  Gtk::Grid dmx_page_;
  struct UniverseListColumns : public Gtk::TreeModelColumnRecord {
    UniverseListColumns() {
      add(universe_);
      add(type_);
      add(ola_universe_);
      add(description_);
    }

    Gtk::TreeModelColumn<int> universe_;
    Gtk::TreeModelColumn<Glib::ustring> type_;
    Gtk::TreeModelColumn<Glib::ustring> ola_universe_;
    Gtk::TreeModelColumn<Glib::ustring> description_;
  } universe_columns_;
  Glib::RefPtr<Gtk::ListStore> universe_list_store_;
  Gtk::TreeView universe_list_view_;
  Gtk::Button reload_ola_button_{"Reload ola"};
  Gtk::Label ola_universe_label_{"Ola universe:"};
  Gtk::ComboBoxText ola_universe_combo_;
  Gtk::RadioButton dmx_none_rb_{"None"};
  Gtk::RadioButton dmx_input_rb_{"Input"};
  Gtk::RadioButton dmx_disconnected_input_rb_{"Disconnected"};
  Gtk::RadioButton dmx_fader_control_rb_{"Fader control"};
  Gtk::RadioButton dmx_merge_rb_{"Merge into output"};
  Gtk::Frame dmx_input_function_frame_{"Function"};
  Gtk::VBox dmx_input_function_box_;

  Gtk::RadioButton dmx_output_rb_{"Output"};

  Gtk::VBox midi_page_;

  RecursionLock recursion_lock_;
};

}  // namespace glight::gui

#endif
