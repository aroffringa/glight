#include "settingswindow.h"

#include <cassert>
#include <variant>

#include <alsa/asoundlib.h>

#include "gui/instance.h"

#include "system/settings.h"

#include "theatre/management.h"

namespace glight::gui::windows {

using theatre::UniverseType;
using theatre::devices::InputMapping;
using theatre::devices::InputMappingFunction;
using theatre::devices::OutputMapping;
using theatre::devices::UniverseMap;

SettingsWindow::SettingsWindow() {
  MakeDmxPage();

  // notebook_.append_page(midi_page_, "MIDI");

  MakeAudioPage();

  add(notebook_);
  notebook_.show_all();

  FillUniverses();
  UpdateAfterSelection();
}

void SettingsWindow::MakeDmxPage() {
  universe_list_store_ = Gtk::ListStore::create(universe_columns_);
  universe_list_view_.set_model(universe_list_store_);
  universe_list_view_.append_column("Universe", universe_columns_.universe_);
  universe_list_view_.append_column("Type", universe_columns_.type_);
  universe_list_view_.append_column("Ola", universe_columns_.ola_universe_);
  universe_list_view_.append_column("Description",
                                    universe_columns_.description_);
  universe_list_view_.set_size_request(100, 100);
  universe_list_view_.get_selection()->signal_changed().connect(
      [&]() { UpdateAfterSelection(); });
  universe_list_view_.set_hexpand(true);
  universe_list_view_.set_vexpand(true);
  dmx_page_.attach(universe_list_view_, 0, 0, 2, 1);
  reload_ola_button_.signal_clicked().connect([&]() { ReloadOla(); });
  dmx_page_.attach(reload_ola_button_, 0, 1, 1, 1);

  dmx_page_.attach(ola_universe_label_, 0, 2, 1, 1);
  ola_universe_combo_.signal_changed().connect(
      [&]() { SaveSelectedOlaUniverse(); });
  dmx_page_.attach(ola_universe_combo_, 1, 2, 1, 1);

  auto save_universe = [&]() { SaveSelectedUniverse(); };
  Gtk::RadioButton::Group in_out_group;

  dmx_none_rb_.set_group(in_out_group);
  dmx_none_rb_.signal_clicked().connect(save_universe);
  dmx_page_.attach(dmx_none_rb_, 0, 3, 2, 1);

  // DMX input settings
  dmx_input_rb_.set_group(in_out_group);
  dmx_input_rb_.signal_clicked().connect(save_universe);
  dmx_page_.attach(dmx_input_rb_, 0, 4, 2, 1);
  Gtk::RadioButton::Group input_function_group;
  dmx_disconnected_input_rb_.set_group(input_function_group);
  dmx_disconnected_input_rb_.signal_clicked().connect(save_universe);
  dmx_input_function_box_.pack_start(dmx_disconnected_input_rb_);
  dmx_fader_control_rb_.set_group(input_function_group);
  dmx_fader_control_rb_.signal_clicked().connect(save_universe);
  dmx_input_function_box_.pack_start(dmx_fader_control_rb_);
  dmx_merge_rb_.set_group(input_function_group);
  dmx_merge_rb_.signal_clicked().connect(save_universe);
  dmx_input_function_box_.pack_start(dmx_merge_rb_);
  dmx_input_function_frame_.add(dmx_input_function_box_);
  dmx_input_function_frame_.set_hexpand(true);
  dmx_page_.attach(dmx_input_function_frame_, 1, 5, 1, 1);

  // DMX output settings
  dmx_output_rb_.set_group(in_out_group);
  dmx_output_rb_.signal_clicked().connect(save_universe);
  dmx_page_.attach(dmx_output_rb_, 0, 6, 2, 1);
  notebook_.append_page(dmx_page_, "DMX");
}

void SettingsWindow::MakeAudioPage() {
  void** hints = nullptr;
  if (snd_device_name_hint(-1, "pcm", &hints) >= 0) {
    size_t hi = 0;
    output_devices_combo_.remove_all();
    const std::string selected_input = Instance::Settings().audio_input;
    const std::string selected_output = Instance::Settings().audio_output;
    while (hints[hi] != nullptr) {
      char* device_name = snd_device_name_get_hint(hints[hi], "NAME");
      char* device_desc = snd_device_name_get_hint(hints[hi], "DESC");
      char* input_or_output = snd_device_name_get_hint(hints[hi], "IOID");
      const bool is_input = input_or_output == nullptr ||
                            std::strcmp(input_or_output, "Input") == 0;
      const bool is_output = input_or_output == nullptr ||
                             std::strcmp(input_or_output, "Output") == 0;
      assert(is_input || is_output);
      std::string device_desc_str(device_desc);
      const std::size_t new_line = device_desc_str.find('\n');
      if (new_line != std::string::npos) {
        device_desc_str.resize(new_line);
      }
      const std::string description_with_name =
          device_desc_str + " (" + device_name + ")";
      if (is_input) {
        input_devices_combo_.append(description_with_name);
        input_devices_.emplace_back(device_name);
      }
      if (is_output) {
        output_devices_combo_.append(description_with_name);
        output_devices_.emplace_back(device_name);
      }
      if (selected_input == device_name) input_devices_combo_.set_active(hi);
      if (selected_output == device_name) output_devices_combo_.set_active(hi);
      free(input_or_output);
      free(device_desc);
      free(device_name);
      ++hi;
    }
    snd_device_name_free_hint(hints);
  }
  input_devices_combo_.signal_changed().connect([&]() { SetInputAudio(); });
  output_devices_combo_.signal_changed().connect([&]() { SetOutputAudio(); });
  audio_page_label_.set_line_wrap(true);
  audio_page_label_.set_max_width_chars(40);
  audio_page_label_.set_margin_start(8);
  audio_page_label_.set_margin_end(8);
  audio_page_label_.set_margin_top(8);
  audio_page_label_.set_margin_bottom(8);
  audio_page_.attach(audio_page_label_, 0, 0, 2, 1);
  audio_page_.attach(input_devices_label_, 0, 1);
  audio_page_.attach(input_devices_combo_, 1, 1);
  audio_page_.attach(output_devices_label_, 0, 2);
  audio_page_.attach(output_devices_combo_, 1, 2);
  notebook_.append_page(audio_page_, "Audio");
}

void SettingsWindow::FillUniverses() {
  universe_list_store_->clear();
  theatre::Management& management = Instance::Management();
  const UniverseMap& universes = management.GetUniverses();
  const size_t n_universes = universes.NUniverses();
  for (size_t universe = 0; universe != n_universes; ++universe) {
    Gtk::TreeRow new_row = *universe_list_store_->append();
    SetUniverseRow(universes, universe, new_row);
  }
}

void SettingsWindow::SetUniverseRow(const UniverseMap& universes,
                                    size_t universe, Gtk::TreeRow& row) {
  row[universe_columns_.universe_] = universe;

  const UniverseType type = universes.GetUniverseType(universe);
  std::string description;
  switch (type) {
    case UniverseType::Input: {
      row[universe_columns_.type_] = "Input";
      const theatre::devices::InputMapping& mapping =
          universes.GetInputMapping(universe);
      if (mapping.ola_universe) {
        row[universe_columns_.ola_universe_] =
            std::to_string(*mapping.ola_universe);
      } else {
        row[universe_columns_.ola_universe_] = "-";
      }
      switch (mapping.function) {
        case InputMappingFunction::NoFunction:
          if (mapping.ola_universe)
            description = "Disconnected Ola input universe";
          else
            description = "Disconnected dummy input";
          break;
        case InputMappingFunction::FaderControl:
          description = "Fader control";
          break;
        case InputMappingFunction::Merge:
          description =
              "Merge with output " + std::to_string(*mapping.merge_universe);
          break;
      }
    } break;
    case UniverseType::Output:
      row[universe_columns_.type_] = "Output";
      if (universes.GetOutputMapping(universe).ola_universe) {
        row[universe_columns_.ola_universe_] =
            std::to_string(*universes.GetOutputMapping(universe).ola_universe);
        description = "Output to Ola";
      } else {
        row[universe_columns_.ola_universe_] = "-";
        description = "Disconnected dummy output";
      }
      break;
    case UniverseType::Uninitialized:
      row[universe_columns_.type_] = "-";
      break;
  }
  row[universe_columns_.description_] = description;
}

void SettingsWindow::UpdateAfterSelection() {
  RecursionLock::Token token(recursion_lock_);
  Gtk::TreeModel::iterator iter =
      universe_list_view_.get_selection()->get_selected();
  ola_universe_combo_.remove_all();
  if (iter) {
    dmx_none_rb_.set_sensitive(true);
    dmx_input_rb_.set_sensitive(true);
    dmx_output_rb_.set_sensitive(true);

    Gtk::TreeRow row(iter);
    const size_t universe = row[universe_columns_.universe_];
    const theatre::devices::UniverseMap& universes =
        Instance::Management().GetUniverses();
    const UniverseType type = universes.GetUniverseType(universe);
    ola_universe_combo_.append("-");
    std::vector<size_t> ola_universes;
    if (universes.GetOla()) ola_universes = universes.GetOla()->GetUniverses();
    for (size_t u : ola_universes) {
      if (universes.GetOla()->GetUniverseType(u) == type) {
        ola_universe_combo_.append(std::to_string(u));
      }
    }
    switch (type) {
      case UniverseType::Uninitialized:
        dmx_none_rb_.set_active(true);
        dmx_input_function_frame_.set_sensitive(false);
        break;
      case UniverseType::Input:
        dmx_input_rb_.set_active(true);
        dmx_input_function_frame_.set_sensitive(true);

        switch (universes.GetInputMapping(universe).function) {
          case InputMappingFunction::NoFunction:
            dmx_disconnected_input_rb_.set_active(true);
            break;
          case InputMappingFunction::FaderControl:
            dmx_fader_control_rb_.set_active(true);
            break;
          case InputMappingFunction::Merge:
            dmx_merge_rb_.set_active(true);
            break;
        }
        break;
      case UniverseType::Output:
        dmx_output_rb_.set_active(true);
        dmx_input_function_frame_.set_sensitive(false);
        break;
    }
  } else {
    dmx_none_rb_.set_sensitive(false);
    dmx_input_rb_.set_sensitive(false);
    dmx_output_rb_.set_sensitive(false);
    dmx_input_function_frame_.set_sensitive(false);
  }
}

void SettingsWindow::SaveSelectedUniverse() {
  std::lock_guard lock(Instance::Management().Mutex());
  Gtk::TreeModel::iterator iter =
      universe_list_view_.get_selection()->get_selected();
  if (iter && recursion_lock_.IsFirst()) {
    Gtk::TreeRow row(iter);
    const size_t universe_index = row[universe_columns_.universe_];
    theatre::devices::UniverseMap& universes =
        Instance::Management().GetUniverses();
    theatre::devices::UniverseMapping mapping =
        universes.GetMapping(universe_index);
    if (dmx_input_rb_.get_active()) {
      if (!std::holds_alternative<InputMapping>(mapping)) {
        mapping = InputMapping();
      }
      InputMappingFunction function = InputMappingFunction::NoFunction;
      if (dmx_fader_control_rb_.get_active()) {
        function = InputMappingFunction::FaderControl;
      } else if (dmx_merge_rb_.get_active()) {
        function = InputMappingFunction::Merge;
        std::get<InputMapping>(mapping).merge_universe =
            universes.FirstOutputUniverse();
      }
      std::get<InputMapping>(mapping).function = function;
    } else if (dmx_output_rb_.get_active()) {
      if (!std::holds_alternative<theatre::devices::OutputMapping>(mapping)) {
        mapping = theatre::devices::OutputMapping();
      }
    } else {
      mapping = theatre::devices::UniverseMapping();
    }
    universes.SetUniverseMapping(universe_index, mapping);
    SetUniverseRow(universes, universe_index, row);
  }
}

void SettingsWindow::SaveSelectedOlaUniverse() {
  std::lock_guard lock(Instance::Management().Mutex());
  Gtk::TreeModel::iterator iter =
      universe_list_view_.get_selection()->get_selected();
  if (iter && recursion_lock_.IsFirst() && ola_universe_combo_.get_active()) {
    system::OptionalNumber<size_t> ola_universe;
    if (ola_universe_combo_.get_active_text() != "-")
      ola_universe = std::atoi(ola_universe_combo_.get_active_text().c_str());
    Gtk::TreeRow row(iter);
    const size_t universe_index = row[universe_columns_.universe_];
    theatre::devices::UniverseMap& universes =
        Instance::Management().GetUniverses();
    theatre::devices::UniverseMapping mapping =
        universes.GetMapping(universe_index);
    if (std::holds_alternative<InputMapping>(mapping)) {
      std::get<InputMapping>(mapping).ola_universe = ola_universe;
    } else if (std::holds_alternative<OutputMapping>(mapping)) {
      std::get<OutputMapping>(mapping).ola_universe = ola_universe;
    }
    universes.SetUniverseMapping(universe_index, mapping);
    SetUniverseRow(universes, universe_index, row);
  }
}

void SettingsWindow::ReloadOla() {
  std::unique_lock lock(Instance::Management().Mutex());
  Instance::Management().GetUniverses().Open();
  lock.unlock();
  FillUniverses();
  UpdateAfterSelection();
}

void SettingsWindow::SetInputAudio() {
  const std::string& selected_device =
      input_devices_[input_devices_combo_.get_active_row_number()];
  Instance::Settings().audio_input = selected_device;
  Instance::Management().StartBeatFinder();
}

void SettingsWindow::SetOutputAudio() {
  const std::string& selected_device =
      output_devices_[output_devices_combo_.get_active_row_number()];
  Instance::Settings().audio_output = selected_device;
}

}  // namespace glight::gui::windows
