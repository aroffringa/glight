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
  explicit FaderState(theatre::SourceValue *pv);
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

class FaderSetupState {
 public:
  FaderSetupState()
      : isActive(false),
        isSolo(false),
        fadeInSpeed(0),
        fadeOutSpeed(0),
        width(0),
        height(0) {}
  std::string name;
  bool isActive;
  bool isSolo;
  size_t fadeInSpeed, fadeOutSpeed;
  size_t width, height;

  bool IsAssigned(const theatre::SourceValue *p) const {
    for (const class FaderState &fader : faders)
      if (p == fader.GetSourceValue()) return true;
    return false;
  }

  void ChangeManagement(theatre::Management &management);

  std::vector<FaderState> faders;
};

class GUIState {
 public:
  std::vector<std::unique_ptr<FaderSetupState>> &FaderSetups() {
    return _faderSetups;
  }
  const std::vector<std::unique_ptr<FaderSetupState>> &FaderSetups() const {
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
    for (const std::unique_ptr<FaderSetupState> &fader : _faderSetups) {
      if (fader->IsAssigned(s)) return true;
    }
    return false;
  }

  void ChangeManagement(theatre::Management &management) {
    for (std::unique_ptr<FaderSetupState> &fader : _faderSetups)
      fader->ChangeManagement(management);
  }

 private:
  sigc::signal<void()> _faderSetupSignalChange;
  std::vector<std::unique_ptr<FaderSetupState>> _faderSetups;
};

}  // namespace glight::gui

#endif
