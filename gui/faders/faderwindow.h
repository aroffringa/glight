#ifndef GUI_FADER_WINDOW_H_
#define GUI_FADER_WINDOW_H_

#include <giomm/simpleaction.h>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/dialog.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/grid.h>
#include <gtkmm/image.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scale.h>
#include <gtkmm/window.h>

#include <chrono>
#include <optional>

#include "../../theatre/forwards.h"

#include "system/midi/manager.h"

#include "../recursionlock.h"

namespace glight::gui {

class ControlMenu;
class ControlWidget;
class EventTransmitter;
class FaderState;
class FaderSetState;
class GUIState;

class FaderWindow : public Gtk::Window {
 public:
  /**
   * Construct a fader window with a new, empty fader setup.
   */
  FaderWindow(size_t keyRowIndex);

  ~FaderWindow();

  void LoadNew();
  void LoadState(FaderSetState *state);

  /**
   * Set all sliders to the source values
   */
  void UpdateValues();

  bool HandleKeyDown(char key);
  bool HandleKeyUp(char key);
  bool IsAssigned(theatre::SourceValue *presetValue) const;
  size_t KeyRowIndex() const { return _keyRowIndex; }

  FaderSetState *State() { return _state; }

  /// The fader menu is stored here so that only one menu is allocated at one
  /// time (instead of each fader allocating its own menu)
  std::unique_ptr<ControlMenu> &GetControlMenu();

  void SetMidiManager(system::midi::Manager &manager) {
    _connectedMidiManager = &manager;
  }

 private:
  std::string GetLayout() const {
    Glib::ustring state;
    layout_action_->get_state(state);
    return state;
  }
  bool GetSolo() const {
    bool state;
    solo_action_->get_state(state);
    return state;
  }
  int GetFadeInValue() const {
    int state;
    fade_in_action_->get_state(state);
    return state;
  }
  int GetFadeOutValue() const {
    int state;
    fade_out_action_->get_state(state);
    return state;
  }

  void initializeWidgets();
  void initializeMenu();

  void onAddFaderClicked();
  void onAdd5FadersClicked();
  void onAdd5ToggleControlsClicked();
  void onAddToggleClicked();
  void onAddToggleColumnClicked();
  void onAddColorButtonClicked();
  void onAddComboButtonClicked();
  void onAddMoverButtonClicked();
  void removeFader();
  void onRemoveFaderClicked() {
    if (!_upperControls.empty()) removeFader();
  }
  void onRemove5FadersClicked() {
    for (size_t i = 0; i != 5; ++i) onRemoveFaderClicked();
  }
  void onLayoutChanged();
  void onAssignClicked();
  void onAssignChasesClicked();
  void unassign();
  void onSoloToggled();
  void onSetNameClicked();
  void onControlValueChanged(ControlWidget *widget);
  void onControlAssigned(size_t widgetIndex);
  void SaveSize();
  void onChangeUpSpeed();
  void onChangeDownSpeed();
  bool onTimeout() {
    UpdateValues();
    return true;
  }
  void onCrossFaderChange();
  void onStartCrossFader();
  void onInputDeviceClicked();
  void AssignTopToBottom();
  void FlipCrossFader();
  void CrossFadeImmediately();

  void addControl(FaderState &state, bool isUpper);
  void addControlInLayout(FaderState &state) {
    addControl(state, true);
    if (GetLayout() == "dual") addControl(state, false);
  }
  void loadState();
  /**
   * Returns a list with indices to controls that have a
   * default source count of 1.
   */
  std::vector<size_t> SingleSourceControls() const;

  /// The fader menu is stored here so that only one menu is allocated at one
  /// time (instead of each fader allocating its own menu)
  std::unique_ptr<ControlMenu> control_menu_;
  std::shared_ptr<Gio::SimpleAction> layout_action_;
  std::shared_ptr<Gio::SimpleAction> solo_action_;
  std::shared_ptr<Gio::SimpleAction> fade_in_action_;
  std::shared_ptr<Gio::SimpleAction> fade_out_action_;
  size_t _keyRowIndex;

  Gtk::Box _hBox;
  Gtk::Box _leftBox{Gtk::Orientation::VERTICAL};
  Gtk::Grid _controlGrid;
  Gtk::MenuButton _menuButton;

  std::vector<std::unique_ptr<ControlWidget>> _upperControls;
  std::vector<std::unique_ptr<ControlWidget>> _lowerControls;
  std::vector<Gtk::Box> _upperColumns;
  std::vector<Gtk::Box> _lowerColumns;
  FaderSetState *_state = nullptr;
  RecursionLock _recursionLock;
  sigc::connection _timeoutConnection;
  static const char _keyRowsUpper[3][10];
  static const char _keyRowsLower[3][10];
  bool _isCrossFaderStarted = false;
  std::optional<size_t> _connectedInputUniverse;
  system::midi::Manager *_connectedMidiManager = nullptr;
  std::vector<unsigned char> _inputValues;
  std::vector<unsigned char> _previousInputValues;
  // In dual mode
  std::optional<Gtk::Button> _immediateCrossFadeButton;
  std::optional<Gtk::Button> _activateCrossFaderButton;
  std::optional<Gtk::Scale> _crossFader;
  std::unique_ptr<Gtk::Dialog> dialog_;
};

}  // namespace glight::gui

#endif
