#ifndef CONTROLWINDOW_H
#define CONTROLWINDOW_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/grid.h>
#include <gtkmm/liststore.h>
#include <gtkmm/menu.h>
#include <gtkmm/radiomenuitem.h>
#include <gtkmm/scale.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/window.h>

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "../recursionlock.h"

/**
        @author Andre Offringa
*/
class FaderWindow : public Gtk::Window {
 public:
  /**
   * Construct a fader window with a new, empty fader setup.
   */
  FaderWindow(class EventTransmitter &showWindow, class GUIState &guiState,
              class Management &management, size_t keyRowIndex);

  ~FaderWindow();

  void LoadNew();
  void LoadState(class FaderSetupState *state);

  /**
   * Set all sliders to the preset values
   */
  void ReloadValues();

  bool HandleKeyDown(char key);
  bool HandleKeyUp(char key);
  bool IsAssigned(class SourceValue *presetValue);
  size_t KeyRowIndex() const { return _keyRowIndex; }

  /**
   * Associates all faders to a different management instance.
   *
   * Faders will be assigned to the presets of the new management instance, by
   * looking up the preset by Id. This method is e.g. used for switching between
   * dry and real mode. Faders that are assigned to a preset with an Id that
   * does not correspond to a preset in the new instance are unassigned.
   */
  void ChangeManagement(class Management &management, bool moveSliders);

  FaderSetupState *State() { return _state; }

 private:
  void initializeWidgets();
  void initializeMenu();

  void onAddFaderClicked() { addControl(false, false); }
  void onAdd5FadersClicked() {
    for (size_t i = 0; i != 5; ++i) addControl(false, false);
  }
  void onAdd5ToggleControlsClicked() {
    for (size_t i = 0; i != 5; ++i) addControl(true, false);
  }
  void onAddToggleClicked() { addControl(true, false); }
  void onAddToggleColumnClicked() { addControl(true, true); }
  void removeFader();
  void onRemoveFaderClicked() {
    if (!_controls.empty()) removeFader();
  }
  void onRemove5FadersClicked() {
    for (size_t i = 0; i != 5; ++i) onRemoveFaderClicked();
  }
  bool onMenuButtonClicked(GdkEventButton *event);
  void onAssignClicked();
  void onAssignChasesClicked();
  void onClearClicked();
  void onSoloToggled();
  void onNameButtonClicked();
  void onNewFaderSetupButtonClicked();
  void onControlValueChanged(double newValue, class ControlWidget *widget);
  void onControlAssigned(size_t widgetIndex);
  bool onResize(GdkEventConfigure *event);
  double mapSliderToSpeed(int sliderVal);
  std::string speedLabel(int value);
  void onChangeUpSpeed();
  void onChangeDownSpeed();
  bool onTimeout() {
    updateValues();
    return true;
  }

  void addControl(bool isToggle, bool newToggleColumn);

  void onFaderSetupChanged();
  void updateFaderSetupList();
  void loadState();
  void updateValues();
  size_t getFadeInSpeed() const;
  size_t getFadeOutSpeed() const;

  class Management *_management;
  size_t _keyRowIndex;

  Gtk::VBox _vBox;
  Gtk::Label _faderSetupLabel;
  Gtk::ComboBox _faderSetup;
  Glib::RefPtr<Gtk::ListStore> _faderSetupList;
  Gtk::HBox _hBoxUpper;
  Gtk::Grid _controlGrid;
  Gtk::Button _nameButton, _newFaderSetupButton;
  Gtk::Button _menuButton;

  Gtk::Menu _popupMenu, _fadeInMenu, _fadeOutMenu;
  Gtk::CheckMenuItem _miSolo;
  Gtk::MenuItem _miFadeIn, _miFadeOut;
  Gtk::RadioMenuItem _miFadeInOption[11], _miFadeOutOption[11];
  Gtk::SeparatorMenuItem _miSep1;
  Gtk::MenuItem _miAssign, _miAssignChases, _miClear;
  Gtk::SeparatorMenuItem _miSep2;
  Gtk::MenuItem _miAddFader, _miAdd5Faders, _miAddToggleButton,
      _miAdd5ToggleButtons, _miAddToggleColumn, _miRemoveFader,
      _miRemove5Faders;

  std::vector<std::unique_ptr<class ControlWidget>> _controls;
  std::vector<Gtk::VBox> _toggleColumns;
  class EventTransmitter &_eventHub;
  class GUIState &_guiState;
  class FaderSetupState *_state;
  RecursionLock _recursionLock;
  sigc::connection _faderSetupChangeConnection, _timeoutConnection;
  static const char _keyRowsUpper[3][10], _keyRowsLower[3][10];

  boost::posix_time::ptime _lastUpdateTime;

  class FaderSetupColumns : public Gtk::TreeModel::ColumnRecord {
   public:
    FaderSetupColumns() {
      add(_obj);
      add(_name);
    }

    Gtk::TreeModelColumn<class FaderSetupState *> _obj;
    Gtk::TreeModelColumn<Glib::ustring> _name;
  } _faderSetupColumns;
};

#endif
