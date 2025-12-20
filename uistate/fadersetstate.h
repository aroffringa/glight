#ifndef GLIGHT_FADER_SET_STATE_H_
#define GLIGHT_FADER_SET_STATE_H_

#include <algorithm>
#include <memory>

#include "faderstate.h"

namespace glight::theatre {
class SourceValue;
}  // namespace glight::theatre

namespace glight::uistate {

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

  bool IsAssigned(const theatre::SourceValue *source) const {
    for (const std::unique_ptr<FaderState> &fader : faders) {
      const std::vector<theatre::SourceValue *> &sources =
          fader->GetSourceValues();
      if (std::find(sources.begin(), sources.end(), source) != sources.end())
        return true;
    }
    return false;
  }

  FaderState &AddState(FaderControlType fader_type, bool new_column) {
    FaderState &state = *faders.emplace_back(std::make_unique<FaderState>());
    state.SetFaderType(fader_type);
    state.SetColumn(new_column);
    return state;
  }

  /**
   * Returns the first unassigned controller. It only considers the types fader
   * or toggle buttons.
   */
  FaderState *GetFirstUnassigned() {
    for (std::unique_ptr<FaderState> &fader : faders) {
      if (fader->GetSourceValues().empty() &&
          (fader->GetFaderType() == FaderControlType::Fader ||
           fader->GetFaderType() == FaderControlType::ToggleButton))
        return fader.get();
    }
    return nullptr;
  }

  std::vector<std::unique_ptr<FaderState>> faders;
};

}  // namespace glight::uistate

#endif
