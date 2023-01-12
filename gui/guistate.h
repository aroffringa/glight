#ifndef GUI_GUI_STATE_H_
#define GUI_GUI_STATE_H_

#include <memory>
#include <string>
#include <vector>

#include <sigc++/connection.h>
#include <sigc++/signal.h>

namespace glight::theatre {
class Management;
class SourceValue;
}  // namespace glight::theatre

namespace glight::gui {

class FaderState {
 public:
  explicit FaderState(theatre::SourceValue *sourceValue);
  FaderState()
      : _sourceValue(nullptr),
        _isToggleButton(false),
        _newToggleButtonColumn(false) {}
  FaderState(const FaderState &source);

  FaderState &operator=(const FaderState &rhs);

  ~FaderState() { _sourceValueDeletedConnection.disconnect(); }
  void SetSourceValue(theatre::SourceValue *value);
  void SetNoSourceValue() { SetSourceValue(nullptr); }
  void SetIsToggleButton(bool isToggle) { _isToggleButton = isToggle; }
  void SetNewToggleButtonColumn(bool newColumn) {
    _newToggleButtonColumn = newColumn;
  }

  // This might return a nullptr to indicate an unset control.
  theatre::SourceValue *GetSourceValue() const { return _sourceValue; }
  bool IsToggleButton() const { return _isToggleButton; }
  bool NewToggleButtonColumn() const { return _newToggleButtonColumn; }

 private:
  void onPresetValueDeleted();
  theatre::SourceValue *_sourceValue;
  bool _isToggleButton;
  bool _newToggleButtonColumn;
  sigc::connection _sourceValueDeletedConnection;
};

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
    for (const class FaderState &fader : faders)
      if (p == fader.GetSourceValue()) return true;
    return false;
  }

  std::vector<FaderState> faders;
};

class GUIState {
 public:
  std::vector<std::unique_ptr<FaderSetState>> &FaderSets() {
    return _faderSetups;
  }
  const std::vector<std::unique_ptr<FaderSetState>> &FaderSetups() const {
    return _faderSetups;
  }

  /**
   * Emitted when fadersetupstates are added or removed from the list.
   */
  sigc::signal<void()> &FaderSetupSignalChange() {
    return _faderSetupSignalChange;
  }

  void EmitFaderSetupChangeSignal() { _faderSetupSignalChange(); }

  void Clear() { _faderSetups.clear(); }

  bool Empty() const { return _faderSetups.empty(); }

  bool IsAssigned(const theatre::SourceValue *s) const {
    for (const std::unique_ptr<FaderSetState> &fader : _faderSetups) {
      if (fader->IsAssigned(s)) return true;
    }
    return false;
  }

 private:
  sigc::signal<void()> _faderSetupSignalChange;
  std::vector<std::unique_ptr<FaderSetState>> _faderSetups;
};

}  // namespace glight::gui

#endif
