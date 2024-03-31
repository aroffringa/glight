#include "settingswindow.h"

#include "gui/instance.h"

#include "theatre/management.h"
#include "theatre/devices/universemap.h"

namespace glight::gui {

SettingsWindow::SettingsWindow() {
  universe_list_store_ = Gtk::ListStore::create(universe_columns_);
  universe_list_view_.set_model(universe_list_store_);
  universe_list_view_.append_column("Universe", universe_columns_.universe_);
  universe_list_view_.append_column("Type", universe_columns_.type_);
  universe_list_view_.append_column("Ola", universe_columns_.ola_universe_);
  universe_list_view_.append_column("Description",
                                    universe_columns_.description_);
  universe_list_view_.set_size_request(100, 100);
  dmx_page_.pack_start(universe_list_view_, true, true, 0);

  Gtk::RadioButton::Group in_out_group;

  // DMX input settings
  dmx_input_rb_.set_group(in_out_group);
  dmx_page_.pack_start(dmx_input_rb_, false, false, 0);
  Gtk::RadioButton::Group input_function_group;
  dmx_dummy_input_rb_.set_group(input_function_group);
  dmx_input_function_box_.pack_start(dmx_dummy_input_rb_);
  dmx_fader_control_rb_.set_group(input_function_group);
  dmx_input_function_box_.pack_start(dmx_fader_control_rb_);
  dmx_merge_rb_.set_group(input_function_group);
  dmx_input_function_box_.pack_start(dmx_merge_rb_);
  dmx_input_function_frame_.add(dmx_input_function_box_);

  // DMX output settings
  dmx_page_.pack_start(dmx_input_function_frame_, false, false, 0);
  dmx_output_rb_.set_group(in_out_group);
  dmx_page_.pack_start(dmx_output_rb_, false, false, 0);
  notebook_.append_page(dmx_page_, "DMX");

  notebook_.append_page(midi_page_, "MIDI");

  add(notebook_);
  notebook_.show_all();

  FillUniverses();
}

void SettingsWindow::FillUniverses() {
  universe_list_store_->clear();
  theatre::Management& management = Instance::Management();
  theatre::devices::UniverseMap& universes = management.GetUniverses();
  const size_t n_universes = universes.NUniverses();
  for (size_t universe = 0; universe != n_universes; ++universe) {
    Gtk::TreeRow new_row = *universe_list_store_->append();
    new_row[universe_columns_.universe_] = universe;

    const theatre::UniverseType type = universes.GetUniverseType(universe);
    std::string description;
    switch (type) {
      case theatre::UniverseType::Input: {
        new_row[universe_columns_.type_] = "Input";
        const theatre::devices::InputMapping& mapping =
            universes.GetInputMapping(universe);
        if (mapping.ola_universe) {
          new_row[universe_columns_.ola_universe_] =
              std::to_string(*mapping.ola_universe);
          switch (mapping.function) {
            case theatre::devices::InputMappingFunction::NoFunction:
              description = "disconnected ola input Universe";
              break;
            case theatre::devices::InputMappingFunction::FaderControl:
              description = "fader control";
              break;
            case theatre::devices::InputMappingFunction::Merge:
              description = "merge with output " +
                            std::to_string(*mapping.merge_universe);
              break;
          }
        } else {
          new_row[universe_columns_.ola_universe_] = "-";
          description = "disconnected dummy input";
        }
      } break;
      case theatre::UniverseType::Output:
        new_row[universe_columns_.type_] = "Output";
        if (universes.GetOutputMapping(universe).ola_universe) {
          new_row[universe_columns_.ola_universe_] = std::to_string(
              *universes.GetOutputMapping(universe).ola_universe);
        }
        break;
      case theatre::UniverseType::Uninitialized:
        new_row[universe_columns_.type_] = "-";
        break;
    }
    new_row[universe_columns_.description_] = description;
  }
}

}  // namespace glight::gui
