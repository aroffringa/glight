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

class ControlWidget;
class EventTransmitter;
class FaderSetupState;
class GUIState;

class FaderWindow : public Gtk::Window {
 public:
  /**
   * Construct a fader window with a new, empty fader setup.
   */
  FaderWindow(EventTransmitter &showWindow, GUIState &guiState,
              theatre::Management &management, size_t keyRowIndex);

  ~FaderWindow();

  void LoadNew();
  void LoadState(FaderSetupState *state);

  /**
   * Set all sliders to the preset values
   */
  void ReloadValues();

  bool HandleKeyDown(char key);
  bool HandleKeyUp(char key);
  bool IsAssigned(theatre::SourceValue *presetValue) const;
  size_t KeyRowIndex() const { return _keyRowIndex; }

  FaderSetupState *State() { return _state; }

 private:
  void initializeWidgets();
  void initializeMenu();

  void onAddFaderClicked() { addControlInLayout(false, false); }
  void onAdd5FadersClicked() {
    for (size_t i = 0; i != 5; ++i) addControlInLayout(false, false);
  }
  void onAdd5ToggleControlsClicked() {
    for (size_t i = 0; i != 5; ++i) addControlInLayout(true, false);
  }
  void onAddToggleClicked() { addControlInLayout(true, false); }
  void onAddToggleColumnClicked() { addControlInLayout(true, true); }
  void removeFader();
  void onRemoveFaderClicked() {
    if (!_upperControls.empty()) removeFader();
  }
  void onRemove5FadersClicked() {
    for (size_t i = 0; i != 5; ++i) onRemoveFaderClicked();
  }
  void onLayoutChanged() {
    RecursionLock::Token token(_recursionLock);
    loadState();
  }
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
    ReloadValues();
    return true;
  }
  void onCrossFaderChange();
  void onRunCrossFader();
  void FlipCrossFader();

  void addControl(bool isToggle, bool newToggleColumn, bool isPrimary);
  void addControlInLayout(bool isToggle, bool newToggleColumn) {
    addControl(isToggle, newToggleColumn, true);
    if (_miDualLayout.get_active())
      addControl(isToggle, newToggleColumn, false);
  }
  void loadState();
  size_t getFadeInSpeed() const;
  size_t getFadeOutSpeed() const;

  theatre::Management *_management;
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
  FaderSetupState *_state;
  RecursionLock _recursionLock;
  sigc::connection _timeoutConnection;
  std::optional<Gtk::Button> _activateCrossFaderButton;
  std::optional<Gtk::VScale> _crossFader;
  static const char _keyRowsUpper[3][10], _keyRowsLower[3][10];
};

}  // namespace glight::gui

#endif
