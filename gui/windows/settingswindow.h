#ifndef GUI_SETTINGS_WINDOW_H_
#define GUI_SETTINGS_WINDOW_H_

#include <gtkmm/box.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/notebook.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

namespace glight::gui {

class SettingsWindow : public Gtk::Window {
 public:
  SettingsWindow();

  void SetPointer(std::unique_ptr<SettingsWindow> self) {
    self_ = std::move(self);
  }

 private:
  void FillUniverses();
  std::unique_ptr<SettingsWindow> self_;
  Gtk::Notebook notebook_;
  Gtk::VBox dmx_page_;
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
  Gtk::RadioButton dmx_input_rb_{"Input"};
  Gtk::RadioButton dmx_dummy_input_rb_{"Dummy input"};
  Gtk::RadioButton dmx_fader_control_rb_{"Fader control"};
  Gtk::RadioButton dmx_merge_rb_{"Merge into output"};
  Gtk::Frame dmx_input_function_frame_{"Function"};
  Gtk::VBox dmx_input_function_box_;

  Gtk::RadioButton dmx_output_rb_{"Output"};
  Glib::RefPtr<Gtk::ListStore> universe_list_store_;
  Gtk::TreeView universe_list_view_;

  Gtk::VBox midi_page_;
};

}  // namespace glight::gui

#endif
