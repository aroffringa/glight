#ifndef GLIGHT_FADER_SET_STATE_H_
#define GLIGHT_FADER_SET_STATE_H_

#include <memory>

#include "faderstate.h"

namespace glight::theatre {
class SourceValue;
}  // namespace glight::theatre

namespace glight::gui {

enum class FaderSetMode { Primary, Secondary, Dual };

inline std::string ToString(FaderSetMode mode) {
  switch (mode) {
    case FaderSetMode::Primary:
      return "primary";
    case FaderSetMode::Secondary:
      return "secondary";
    case FaderSetMode::Dual:
      return "dual";
  }
  return {};
}

inline FaderSetMode ToFaderSetMode(const std::string &mode_str) {
  if (mode_str == "secondary")
    return FaderSetMode::Secondary;
  else if (mode_str == "dual")
    return FaderSetMode::Dual;
  else
    return FaderSetMode::Primary;  // also the default
}

class FaderSetState {
 public:
  FaderSetState() = default;

  FaderSetMode mode = FaderSetMode::Primary;
  std::string name;
  bool isActive = false;
  bool isSolo = false;
  // 0 (fastest) -- 10 (slowest)
  size_t fadeInSpeed = 7;
  size_t fadeOutSpeed = 7;
  size_t width = 0;
  size_t height = 0;

  bool IsAssigned(const theatre::SourceValue *p) const {
    for (const std::unique_ptr<FaderState> &fader : faders)
      if (p == fader->GetSourceValue()) return true;
    return false;
  }

  FaderState &AddState(bool is_toggle_button, bool new_toggle_column) {
    FaderState &state = *faders.emplace_back(std::make_unique<FaderState>());
    state.SetIsToggleButton(is_toggle_button);
    state.SetNewToggleButtonColumn(new_toggle_column);
    return state;
  }

  std::vector<std::unique_ptr<FaderState>> faders;
};

}  // namespace glight::gui

#endif
