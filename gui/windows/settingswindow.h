#ifndef GUI_SETTINGS_WINDOW_H_
#define GUI_SETTINGS_WINDOW_H_

#include <gtkmm/box.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/frame.h>
#include <gtkmm/grid.h>
#include <gtkmm/liststore.h>
#include <gtkmm/notebook.h>
#include <gtkmm/treeview.h>

#include "gui/recursionlock.h"
#include "gui/windows/childwindow.h"

#include "theatre/devices/universemap.h"

namespace glight::gui::windows {

class SettingsWindow : public ChildWindow {
 public:
  SettingsWindow();

 private:
  void FillUniverses();
  void SetUniverseRow(const theatre::devices::UniverseMap& universes,
                      size_t universe, Gtk::TreeRow& row);
  void UpdateAfterSelection();
  void SaveSelectedUniverse();
  void SaveSelectedOlaUniverse();
  void ReloadOla();

  void SetInputAudio();
  void SetOutputAudio();

  void MakeDmxPage();
  void MakeAudioPage();

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
  Gtk::CheckButton dmx_none_rb_{"None"};
  Gtk::CheckButton dmx_input_rb_{"Input"};
  Gtk::CheckButton dmx_disconnected_input_rb_{"Disconnected"};
  Gtk::CheckButton dmx_fader_control_rb_{"Fader control"};
  Gtk::CheckButton dmx_merge_rb_{"Merge into output"};
  Gtk::Frame dmx_input_function_frame_{"Function"};
  Gtk::Box dmx_input_function_box_{Gtk::Orientation::VERTICAL};

  Gtk::CheckButton dmx_output_rb_{"Output"};

  Gtk::Box midi_page_{Gtk::Orientation::VERTICAL};

  std::vector<std::string> input_devices_;
  std::vector<std::string> output_devices_;
  Gtk::Grid audio_page_;
  Gtk::Label audio_page_label_{
      "Select the Alsa device names to be used. The input device is used for "
      "beat detection and volume effects. The output device is used for "
      "playing the audio of scenes."};
  Gtk::Label input_devices_label_{"Input device: "};
  Gtk::ComboBoxText input_devices_combo_;
  Gtk::Label output_devices_label_{"Output device: "};
  Gtk::ComboBoxText output_devices_combo_;

  RecursionLock recursion_lock_;
};

}  // namespace glight::gui::windows

#endif
