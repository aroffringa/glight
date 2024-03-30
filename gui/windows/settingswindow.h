#ifndef GUI_SETTINGS_WINDOW_H_
#define GUI_SETTINGS_WINDOW_H_

#include <gtkmm/box.h>
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
  std::unique_ptr<SettingsWindow> self_;
  Gtk::Notebook notebook_;
  Gtk::VBox dmx_page_;
  Gtk::Frame dmx_input_frame_{"Input device"};
  struct FixturesListColumns : public Gtk::TreeModelColumnRecord {
    FixturesListColumns() {
      add(universe_);
      add(description_);
    }

    Gtk::TreeModelColumn<int> universe_;
    Gtk::TreeModelColumn<Glib::ustring> description_;
  } dmx_input_columns_;
  Glib::RefPtr<Gtk::ListStore> dmx_input_list_store_;
  Gtk::TreeView dmx_input_list_view_;
  Gtk::Label dmx_output_label_{"Output device"};

  Gtk::VBox midi_page_;
};

}  // namespace glight::gui

#endif
