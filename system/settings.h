#ifndef GLIGHT_SYSTEM_SETTINGS_H_
#define GLIGHT_SYSTEM_SETTINGS_H_

#include <string>
#include <vector>

namespace glight::system {

struct Settings {
  std::string audio_input = "default";
  std::string audio_output = "default";
};

Settings LoadSettings();

void Save(const Settings& settings);

}  // namespace glight::system

#endif
