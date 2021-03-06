#ifndef GUI_STATE_H
#define GUI_STATE_H

#include <memory>
#include <string>
#include <vector>

#include <sigc++/connection.h>
#include <sigc++/signal.h>

class FaderState {
 public:
  explicit FaderState(class SourceValue *pv);
  FaderState()
      : _sourceValue(nullptr),
        _isToggleButton(false),
        _newToggleButtonColumn(false) {}
  FaderState(const FaderState &source);

  FaderState &operator=(const FaderState &rhs);

  ~FaderState() { _presetValueDeletedConnection.disconnect(); }
  void SetSourceValue(class SourceValue *presetValue);
  void SetNoSourceValue() { SetSourceValue(nullptr); }
  void SetIsToggleButton(bool isToggle) { _isToggleButton = isToggle; }
  void SetNewToggleButtonColumn(bool newColumn) {
    _newToggleButtonColumn = newColumn;
  }

  // This might return a nullptr to indicate an unset control.
  class SourceValue *GetSourceValue() const {
    return _sourceValue;
  }
  bool IsToggleButton() const { return _isToggleButton; }
  bool NewToggleButtonColumn() const { return _newToggleButtonColumn; }

 private:
  void onPresetValueDeleted();
  class SourceValue *_sourceValue;
  bool _isToggleButton;
  bool _newToggleButtonColumn;
  sigc::connection _presetValueDeletedConnection;
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

  bool IsAssigned(const class SourceValue *p) const {
    for (const class FaderState &fader : faders)
      if (p == fader.GetSourceValue()) return true;
    return false;
  }

  void ChangeManagement(class Management &management);

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

  bool IsAssigned(const class SourceValue *s) const {
    for (const std::unique_ptr<FaderSetupState> &fader : _faderSetups) {
      if (fader->IsAssigned(s)) return true;
    }
    return false;
  }

  void ChangeManagement(class Management &management) {
    for (std::unique_ptr<FaderSetupState> &fader : _faderSetups)
      fader->ChangeManagement(management);
  }

 private:
  sigc::signal<void()> _faderSetupSignalChange;
  std::vector<std::unique_ptr<FaderSetupState>> _faderSetups;
};

#endif
