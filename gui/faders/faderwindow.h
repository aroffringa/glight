#ifndef GUI_FADER_WINDOW_H_
#define GUI_FADER_WINDOW_H_

#include <gtkmm/box.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/grid.h>
#include <gtkmm/image.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/liststore.h>
#include <gtkmm/menu.h>
#include <gtkmm/radiomenuitem.h>
#include <gtkmm/scale.h>
#include <gtkmm/separatormenuitem.h>
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
  bool onResize(GdkEventConfigure *event);
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
    if (_miDualLayout.get_active()) addControl(state, false);
  }
  void loadState();
  size_t getFadeInSpeed() const;
  size_t getFadeOutSpeed() const;
  /**
   * Returns a list with indices to controls that have a
   * default source count of 1.
   */
  std::vector<size_t> SingleSourceControls() const;

  /// The fader menu is stored here so that only one menu is allocated at one
  /// time (instead of each fader allocating its own menu)
  std::unique_ptr<ControlMenu> control_menu_;
  size_t _keyRowIndex;

  Gtk::HBox _hBox;
  Gtk::VBox _leftBox;
  Gtk::Grid _controlGrid;
  Gtk::MenuButton _menuButton;

  Gtk::Menu _popupMenu, _layoutMenu, _fadeInMenu, _fadeOutMenu;
  Gtk::MenuItem _miLayout{"Layout"};
  Gtk::MenuItem _miFadeIn{"Fade in"};
  Gtk::MenuItem _miFadeOut{"Fade out"};
  Gtk::MenuItem _miName{"Set name..."};
  Gtk::CheckMenuItem _miSolo{"Solo"};
  Gtk::RadioMenuItem _miFadeInOption[11], _miFadeOutOption[11];
  Gtk::SeparatorMenuItem _miSep1;
  Gtk::MenuItem _miAssign{"Assign"};
  Gtk::MenuItem _miAssignChases{"Assign to chases"};
  Gtk::MenuItem _miClear{"Clear"};
  Gtk::SeparatorMenuItem _miSep2;
  Gtk::MenuItem _miAddFader{"Add fader"};
  Gtk::MenuItem _miAdd5Faders{"Add 5 faders"};
  Gtk::MenuItem _miAddToggleButton{"Add toggle control"};
  Gtk::MenuItem _miAdd5ToggleButtons{"Add 5 toggle controls"};
  Gtk::MenuItem _miAddColorButton{"Add color button"};
  Gtk::MenuItem _miAddComboButton{"Add combo button"};
  Gtk::MenuItem _miAddMoverControl{"Add mover control"};
  Gtk::MenuItem _miAddToggleColumn{"Add toggle column"};
  Gtk::MenuItem _miRemoveFader{"Remove 1"};
  Gtk::MenuItem _miRemove5Faders{"Remove 5"};
  Gtk::MenuItem _miInputDevice{"Input device..."};

  // Layout menu
  Gtk::RadioMenuItem _miPrimaryLayout{"Primary"};
  Gtk::RadioMenuItem _miSecondaryLayout{"Secondary"};
  Gtk::RadioMenuItem _miDualLayout{"Dual"};

  std::vector<std::unique_ptr<ControlWidget>> _upperControls;
  std::vector<std::unique_ptr<ControlWidget>> _lowerControls;
  std::vector<Gtk::VBox> _upperColumns;
  std::vector<Gtk::VBox> _lowerColumns;
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
};

}  // namespace glight::gui

#endif
