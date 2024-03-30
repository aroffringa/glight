#include "settingswindow.h"

namespace glight::gui {

SettingsWindow::SettingsWindow() {
  dmx_input_list_store_ = Gtk::ListStore::create(dmx_input_columns_);
  dmx_input_list_view_.set_model(dmx_input_list_store_);
  dmx_input_list_view_.append_column("Universe", dmx_input_columns_.universe_);
  dmx_input_list_view_.append_column("Description",
                                     dmx_input_columns_.description_);
  dmx_input_frame_.add(dmx_input_list_view_);
  dmx_page_.pack_start(dmx_input_frame_, true, true, 0);
  dmx_page_.pack_start(dmx_output_label_, false, false, 0);
  notebook_.append_page(dmx_page_, "DMX");

  notebook_.append_page(midi_page_, "MIDI");

  add(notebook_);
  notebook_.show_all();
}

}  // namespace glight::gui
