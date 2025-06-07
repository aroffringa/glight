#include "settings.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include <glibmm/miscutils.h>

#include "jsonreader.h"
#include "jsonwriter.h"

namespace glight::system {

using json::AssignOptionalString;
using json::Node;
using json::Object;
using json::ToObj;

namespace {

std::string GetConfigDir() {
  return std::filesystem::path(Glib::get_user_config_dir()) / "glight";
}

std::string GetConfigFilename(std::string_view config_dir) {
  return (std::filesystem::path(config_dir) / "config").string();
}

bool TryMakeDir(const std::filesystem::path path) {
  if (!std::filesystem::is_directory(path)) {
    // We don't want to crash if the dir can't be created; we will just report
    // an error to the cmd line
    try {
      std::filesystem::create_directories(path);
    } catch (std::exception& exception) {
      return false;
    }
  }
  return true;
}

}  // namespace

void Save(const Settings& settings) {
  const std::string config_dir = GetConfigDir();
  if (!TryMakeDir(config_dir))
    std::cerr << "Could not create directory for config file (" << config_dir
              << ")\n";
  const std::string config_filename = GetConfigFilename(config_dir);
  std::ofstream file(config_filename);
  if (!file)
    std::cerr << "Error saving config file: " << config_filename << '\n';
  json::JsonWriter writer(file);

  writer.StartObject();
  writer.StartObject("system");
  writer.StartObject("audio");
  writer.String("input", settings.audio_input);
  writer.String("output", settings.audio_output);
  writer.EndObject();  // audio
  writer.EndObject();  // system
  writer.EndObject();  // main
}

void ParseAudio(Settings& settings, const Object& audio) {
  AssignOptionalString(settings.audio_input, audio, "input");
  AssignOptionalString(settings.audio_output, audio, "output");
}

void ParseSystem(Settings& settings, const Object& system) {
  if (system.contains("audio")) {
    ParseAudio(settings, ToObj(system["audio"]));
  }
}

Settings LoadSettings() {
  Settings settings;
  const std::string config_filename = GetConfigFilename(GetConfigDir());
  // If there's no file yet, return default settings without a message
  if (!std::filesystem::exists(config_filename)) return settings;
  std::ifstream file(config_filename);
  if (!file) {
    std::cout << "Could not open configuration file: using default settings.\n";
    return settings;
  }

  try {
    std::unique_ptr<Node> json_node = json::Parse(file);
    const Object& json_config = ToObj(*json_node);
    if (json_config.contains("system")) {
      ParseSystem(settings, ToObj(json_config["system"]));
    }
  } catch (std::exception& exception) {
    std::cerr << "Failed to read configuration file with error '"
              << exception.what() << "': using default settings.\n";
    settings = Settings();
  }
  return settings;
}

}  // namespace glight::system
