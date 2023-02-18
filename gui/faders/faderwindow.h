#ifndef GUI_FADER_WINDOW_H_
#define GUI_FADER_WINDOW_H_

#include <gtkmm/box.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/buttonbox.h>
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
  FaderWindow(EventTransmitter &event_hub, GUIState &guiState,
              theatre::Management &management, size_t keyRowIndex);

  ~FaderWindow();

  void LoadNew();
  void LoadState(FaderSetState *state);

  /**
   * Set all sliders to the preset values
   */
  void UpdateValues();

  bool HandleKeyDown(char key);
  bool HandleKeyUp(char key);
  bool IsAssigned(theatre::SourceValue *presetValue) const;
  size_t KeyRowIndex() const { return _keyRowIndex; }

  FaderSetState *State() { return _state; }

  EventTransmitter& GetEventTransmitter() { return _eventHub; }
  theatre::Management &GetManagement() { return _management; }
  std::unique_ptr<ControlMenu>& GetControlMenu();
 private:
  void initializeWidgets();
  void initializeMenu();

  void onAddFaderClicked();
  void onAdd5FadersClicked();
  void onAdd5ToggleControlsClicked();
  void onAddToggleClicked();
  void onAddToggleColumnClicked();
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
  void onControlValueChanged(double newValue, ControlWidget *widget);
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

  /// The fader menu is stored here so that only one menu is allocated at one time
  /// (instead of each fader allocating its own menu)
  std::unique_ptr<ControlMenu> control_menu_;
  theatre::Management &_management;
  size_t _keyRowIndex;

  Gtk::HBox _hBox;
  Gtk::VBox _leftBox;
  Gtk::Grid _controlGrid;
  Gtk::MenuButton _menuButton;

  Gtk::Image _miNameImage;
  Gtk::Menu _popupMenu, _layoutMenu, _fadeInMenu, _fadeOutMenu;
  Gtk::MenuItem _miLayout, _miFadeIn, _miFadeOut;
  Gtk::ImageMenuItem _miName;
  Gtk::CheckMenuItem _miSolo;
  Gtk::RadioMenuItem _miFadeInOption[11], _miFadeOutOption[11];
  Gtk::SeparatorMenuItem _miSep1;
  Gtk::MenuItem _miAssign, _miAssignChases, _miClear;
  Gtk::SeparatorMenuItem _miSep2;
  Gtk::MenuItem _miAddFader, _miAdd5Faders, _miAddToggleButton,
      _miAdd5ToggleButtons, _miAddToggleColumn, _miRemoveFader,
      _miRemove5Faders;

  // Layout menu
  Gtk::RadioMenuItem _miPrimaryLayout;
  Gtk::RadioMenuItem _miSecondaryLayout;
  Gtk::RadioMenuItem _miDualLayout;

  std::vector<std::unique_ptr<ControlWidget>> _upperControls;
  std::vector<std::unique_ptr<ControlWidget>> _lowerControls;
  std::vector<Gtk::VBox> _upperColumns;
  std::vector<Gtk::VBox> _lowerColumns;
  EventTransmitter &_eventHub;
  GUIState &_guiState;
  FaderSetState *_state;
  RecursionLock _recursionLock;
  sigc::connection _timeoutConnection;
  static const char _keyRowsUpper[3][10], _keyRowsLower[3][10];
  bool _isCrossFaderStarted = false;
  // In dual mode
  std::optional<Gtk::Button> _immediateCrossFadeButton;
  std::optional<Gtk::Button> _activateCrossFaderButton;
  std::optional<Gtk::VScale> _crossFader;
};

}  // namespace glight::gui

#endif
