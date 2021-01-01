#include "faderwindow.h"
#include "faderwidget.h"
#include "togglewidget.h"

#include "../eventtransmitter.h"
#include "../guistate.h"

#include "../../theatre/chase.h"
#include "../../theatre/management.h"
#include "../../theatre/presetvalue.h"

#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include <glibmm/main.h>

const char FaderWindow::_keyRowsUpper[3][10] = {
    {'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?'},
    {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':'},
    {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'}};
const char FaderWindow::_keyRowsLower[3][10] = {
    {'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'},
    {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';'},
    {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'}};

FaderWindow::FaderWindow(EventTransmitter &eventHub, GUIState &guiState,
                         Management &management, size_t keyRowIndex)
    : _management(&management),
      _keyRowIndex(keyRowIndex),
      _faderSetupLabel("Fader setup: "),
      _nameButton("Name"),
      _newFaderSetupButton(Gtk::Stock::NEW),
      _menuButton("Menu"),
      _miSolo("Solo"),
      _miFadeIn("Fade in"),
      _miFadeOut("Fade out"),
      _miAssign("Assign"),
      _miAssignChases("Assign to chases"),
      _miClear("Clear"),
      _miAddFader("Add fader"),
      _miAdd5Faders("Add 5 faders"),
      _miAddToggleButton("Add toggle control"),
      _miAdd5ToggleButtons("Add 5 toggle controls"),
      _miAddToggleColumn("Add toggle column"),
      _miRemoveFader("Remove 1"),
      _miRemove5Faders("Remove 5"),
      _eventHub(eventHub),
      _guiState(guiState),
      _state(nullptr),
      _lastUpdateTime(boost::posix_time::microsec_clock::local_time()) {
  initializeWidgets();
  initializeMenu();
}

FaderWindow::~FaderWindow() {
  _faderSetupChangeConnection.disconnect();
  _state->isActive = false;
  _guiState.EmitFaderSetupChangeSignal();
}

void FaderWindow::LoadNew() {
  _guiState.FaderSetups().emplace_back(new FaderSetupState());
  _state = _guiState.FaderSetups().back().get();
  _state->name = "Unnamed fader setup";
  _state->isActive = true;
  for (size_t i = 0; i != 10; ++i) _state->faders.emplace_back();

  _state->width = std::max(100, get_width());
  _state->height = std::max(300, get_height());
  RecursionLock::Token token(_recursionLock);
  loadState();
  updateFaderSetupList();
}

void FaderWindow::LoadState(class FaderSetupState *state) {
  _state = state;
  _state->isActive = true;
  RecursionLock::Token token(_recursionLock);
  loadState();
  updateFaderSetupList();
}

void FaderWindow::initializeWidgets() {
  set_title("Glight - faders");

  _faderSetupChangeConnection = _guiState.FaderSetupSignalChange().connect(
      sigc::mem_fun(*this, &FaderWindow::updateFaderSetupList));

  signal_configure_event().connect(sigc::mem_fun(*this, &FaderWindow::onResize),
                                   false);

  _timeoutConnection = Glib::signal_timeout().connect(
      sigc::mem_fun(*this, &FaderWindow::onTimeout), 40);

  add(_vBox);

  _vBox.pack_start(_hBoxUpper, false, false);

  _menuButton.set_events(Gdk::BUTTON_PRESS_MASK);
  _menuButton.signal_button_press_event().connect(
      sigc::mem_fun(*this, &FaderWindow::onMenuButtonClicked), false);
  _hBoxUpper.pack_start(_menuButton, false, false, 5);

  _hBoxUpper.pack_start(_faderSetupLabel, false, false);

  _faderSetupList = Gtk::ListStore::create(_faderSetupColumns);
  _faderSetup.set_model(_faderSetupList);
  _faderSetup.pack_start(_faderSetupColumns._name);
  _faderSetup.signal_changed().connect(
      sigc::mem_fun(*this, &FaderWindow::onFaderSetupChanged));

  _hBoxUpper.pack_start(_faderSetup, true, true);

  _nameButton.signal_clicked().connect(
      sigc::mem_fun(*this, &FaderWindow::onNameButtonClicked));
  _nameButton.set_image_from_icon_name("user-bookmarks");
  _hBoxUpper.pack_start(_nameButton, false, false, 5);

  _newFaderSetupButton.signal_clicked().connect(
      sigc::mem_fun(*this, &FaderWindow::onNewFaderSetupButtonClicked));
  _hBoxUpper.pack_start(_newFaderSetupButton, false, false, 5);

  _controlGrid.set_column_spacing(3);
  _vBox.pack_start(_controlGrid, true, true);

  show_all_children();
  set_default_size(0, 300);
}

void FaderWindow::initializeMenu() {
  _miSolo.signal_activate().connect([&]() { onSoloToggled(); });
  _popupMenu.append(_miSolo);

  Gtk::RadioMenuItem::Group fadeInGroup, fadeOutGroup;
  for (size_t i = 0; i != 11; ++i) {
    _miFadeInOption[i].set_label(speedLabel(i));
    _miFadeInOption[i].set_group(fadeInGroup);
    _miFadeInOption[i].signal_activate().connect([&]() { onChangeUpSpeed(); });
    _fadeInMenu.append(_miFadeInOption[i]);

    _miFadeOutOption[i].set_label(speedLabel(i));
    _miFadeOutOption[i].set_group(fadeOutGroup);
    _miFadeOutOption[i].signal_activate().connect(
        [&]() { onChangeDownSpeed(); });
    _fadeOutMenu.append(_miFadeOutOption[i]);
  }
  _miFadeInOption[0].set_active(true);
  _miFadeOutOption[0].set_active(true);

  _miFadeIn.set_submenu(_fadeInMenu);
  _popupMenu.append(_miFadeIn);

  _miFadeOut.set_submenu(_fadeOutMenu);
  _popupMenu.append(_miFadeOut);

  _popupMenu.append(_miSep1);

  _miAssign.signal_activate().connect([&]() { onAssignClicked(); });
  _popupMenu.append(_miAssign);

  _miAssignChases.signal_activate().connect([&]() { onAssignChasesClicked(); });
  _popupMenu.append(_miAssignChases);

  _miClear.signal_activate().connect([&]() { onClearClicked(); });
  _popupMenu.append(_miClear);

  _popupMenu.append(_miSep2);

  _miAddFader.signal_activate().connect([&]() { onAddFaderClicked(); });
  _popupMenu.append(_miAddFader);

  _miAdd5Faders.signal_activate().connect([&]() { onAdd5FadersClicked(); });
  _popupMenu.append(_miAdd5Faders);

  _miAddToggleButton.signal_activate().connect([&]() { onAddToggleClicked(); });
  _popupMenu.append(_miAddToggleButton);

  _miAdd5ToggleButtons.signal_activate().connect(
      [&]() { onAdd5ToggleControlsClicked(); });
  _popupMenu.append(_miAdd5ToggleButtons);

  _miAddToggleColumn.signal_activate().connect(
      [&]() { onAddToggleColumnClicked(); });
  _popupMenu.append(_miAddToggleColumn);

  _miRemoveFader.signal_activate().connect([&]() { onRemoveFaderClicked(); });
  _popupMenu.append(_miRemoveFader);

  _miRemove5Faders.signal_activate().connect(
      [&]() { onRemove5FadersClicked(); });
  _popupMenu.append(_miRemove5Faders);

  _popupMenu.show_all_children();
}

bool FaderWindow::onResize(GdkEventConfigure *event) {
  if (_recursionLock.IsFirst()) {
    _state->height = get_height();
    _state->width = get_width();
  }
  return false;
}

void FaderWindow::addControl(bool isToggle, bool newToggleColumn) {
  if (_toggleColumns.empty()) newToggleColumn = true;
  if (_recursionLock.IsFirst()) {
    _state->faders.emplace_back();
    FaderState &state = _state->faders.back();
    state.SetIsToggleButton(isToggle);
    state.SetNewToggleButtonColumn(newToggleColumn);
  }
  bool hasKey = _controls.size() < 10 && _keyRowIndex < 3;
  char key = hasKey ? _keyRowsLower[_keyRowIndex][_controls.size()] : ' ';

  Gtk::Widget *nameLabel;
  std::unique_ptr<ControlWidget> control;
  if (isToggle) {
    control.reset(new ToggleWidget(*_management, _eventHub, key));
    nameLabel = nullptr;
  } else {
    control.reset(new FaderWidget(*_management, _eventHub, key));
    nameLabel = &static_cast<FaderWidget *>(control.get())->NameLabel();
  }

  control->SetFadeDownSpeed(mapSliderToSpeed(getFadeOutSpeed()));
  control->SetFadeUpSpeed(mapSliderToSpeed(getFadeInSpeed()));
  size_t controlIndex = _controls.size();
  control->SignalValueChange().connect(
      sigc::bind(sigc::mem_fun(*this, &FaderWindow::onControlValueChanged),
                 control.get()));
  control->SignalAssigned().connect(sigc::bind(
      sigc::mem_fun(*this, &FaderWindow::onControlAssigned), controlIndex));

  if (isToggle) {
    if (newToggleColumn) {
      _toggleColumns.emplace_back();
      unsigned hpos = _controls.size() + _toggleColumns.size();
      _controlGrid.attach(_toggleColumns.back(), hpos * 2 + 1, 0, 2, 1);
      _toggleColumns.back().show();
    }
    _toggleColumns.back().pack_start(*control);
  } else {
    unsigned hpos = _controls.size() + _toggleColumns.size();
    _controlGrid.attach(*control, hpos * 2 + 1, 0, 2, 1);

    nameLabel->set_hexpand(true);
    bool even = _controls.size() % 2 == 0;
    _controlGrid.attach(*nameLabel, hpos * 2, even ? 1 : 2, 4, 1);
  }

  control->show();
  _controls.emplace_back(std::move(control));
}

void FaderWindow::removeFader() {
  FaderState &state = _state->faders.back();
  if (state.IsToggleButton() && state.NewToggleButtonColumn())
    _toggleColumns.pop_back();
  _controls.pop_back();
  _state->faders.pop_back();
}

bool FaderWindow::onMenuButtonClicked(GdkEventButton *event) {
  if (event->button == 1) {
    _popupMenu.popup(event->button, event->time);
    return true;
  }
  return false;
}

void FaderWindow::onAssignClicked() {
  for (std::unique_ptr<ControlWidget> &c : _controls) c->Unassign();
  size_t n = _management->SourceValues().size();
  if (!_controls.empty()) {
    size_t controlIndex = 0;
    for (size_t i = 0; i != n; ++i) {
      SourceValue *sv = _management->SourceValues()[i].get();
      if (!_guiState.IsAssigned(sv)) {
        _controls[controlIndex]->Assign(sv, true);
        ++controlIndex;
        if (controlIndex == _controls.size()) break;
      }
    }
  }
}

void FaderWindow::onClearClicked() {
  for (std::unique_ptr<ControlWidget> &c : _controls) c->Unassign();
}

void FaderWindow::onAssignChasesClicked() {
  if (!_controls.empty()) {
    size_t controlIndex = 0;
    for (size_t i = 0; i != _management->SourceValues().size(); ++i) {
      SourceValue *sv = _management->SourceValues()[i].get();
      Chase *c = dynamic_cast<Chase *>(&sv->Controllable());
      if (c != nullptr) {
        _controls[controlIndex]->Assign(sv, true);
        ++controlIndex;
        if (controlIndex == _controls.size()) break;
      }
    }
  }
}

void FaderWindow::onSoloToggled() {
  if (_recursionLock.IsFirst()) {
    _state->isSolo = _miSolo.get_active();
  }
}

void FaderWindow::onControlValueChanged(double newValue,
                                        ControlWidget *widget) {
  if (_miSolo.get_active()) {
    // Limitting the controls might generate another control value change, but
    // since it is an auto generated change we will not apply the limit of that
    // change to other faders.
    if (_recursionLock.IsFirst()) {
      RecursionLock::Token token(_recursionLock);
      double limitValue = ControlWidget::MAX_SCALE_VALUE() - newValue -
                          ControlWidget::MAX_SCALE_VALUE() * 0.01;
      if (limitValue < 0.0) limitValue = 0.0;
      for (std::unique_ptr<ControlWidget> &c : _controls) {
        if (c.get() != widget) c->Limit(limitValue);
      }
    }
  }
}

void FaderWindow::onControlAssigned(size_t widgetIndex) {
  if (_recursionLock.IsFirst())
    _state->faders[widgetIndex].SetSourceValue(
        _controls[widgetIndex]->GetSourceValue());
}

bool FaderWindow::HandleKeyDown(char key) {
  if (_keyRowIndex >= 3) return false;

  for (unsigned i = 0; i < 10; ++i) {
    if (_keyRowsUpper[_keyRowIndex][i] == key) {
      if (i < _controls.size()) {
        _controls[i]->FullOn();
      }
      return true;
    } else if ((_keyRowsLower[_keyRowIndex][i]) == key) {
      if (i < _controls.size()) {
        _controls[i]->Toggle();
      }
      return true;
    }
  }
  return false;
}

bool FaderWindow::HandleKeyUp(char key) {
  if (_keyRowIndex >= 3) return false;

  for (unsigned i = 0; i < 10; ++i) {
    if (_keyRowsUpper[_keyRowIndex][i] == key) {
      if (i < _controls.size()) {
        _controls[i]->FullOff();
      }
      return true;
    }
  }
  return false;
}

bool FaderWindow::IsAssigned(SourceValue *sourceValue) {
  for (std::unique_ptr<ControlWidget> &c : _controls) {
    if (c->GetSourceValue() == sourceValue) return true;
  }
  return false;
}

void FaderWindow::onNameButtonClicked() {
  Gtk::MessageDialog dialog(*this, "Name fader setup", false,
                            Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
  Gtk::Entry entry;
  dialog.get_vbox()->pack_start(entry, Gtk::PACK_SHRINK);
  dialog.get_vbox()->show_all_children();
  dialog.set_secondary_text("Please enter a name for this fader setup");
  int result = dialog.run();
  if (result == Gtk::RESPONSE_OK) {
    _state->name = entry.get_text();
    _guiState.EmitFaderSetupChangeSignal();
  }
}

void FaderWindow::onNewFaderSetupButtonClicked() {
  RecursionLock::Token token(_recursionLock);
  _state->isActive = false;
  _guiState.FaderSetups().emplace_back(new FaderSetupState());
  _state = _guiState.FaderSetups().back().get();
  _state->isActive = true;
  _state->name = "Unnamed fader setup";
  for (size_t i = 0; i != 10; ++i) _state->faders.emplace_back();
  _state->height = 300;
  _state->width = 100;
  loadState();

  token.Release();

  _guiState.EmitFaderSetupChangeSignal();
}

void FaderWindow::updateFaderSetupList() {
  RecursionLock::Token token(_recursionLock);
  _faderSetupList->clear();
  for (const std::unique_ptr<FaderSetupState> &fState :
       _guiState.FaderSetups()) {
    bool itsMe = fState.get() == _state;
    if (!fState->isActive || itsMe) {
      Gtk::TreeModel::iterator row = _faderSetupList->append();
      (*row)[_faderSetupColumns._obj] = fState.get();
      (*row)[_faderSetupColumns._name] = fState->name;
      if (itsMe) {
        _faderSetup.set_active(row);
      }
    }
  }
}

void FaderWindow::onFaderSetupChanged() {
  if (_recursionLock.IsFirst()) {
    RecursionLock::Token token(_recursionLock);
    _state->isActive = false;
    _state->height = get_height();
    _state->width = get_width();
    _state = (*_faderSetup.get_active())[_faderSetupColumns._obj];
    _state->isActive = true;

    loadState();

    token.Release();

    _guiState.EmitFaderSetupChangeSignal();
  }
}

void FaderWindow::loadState() {
  _miSolo.set_active(_state->isSolo);
  _miFadeInOption[_state->fadeInSpeed].set_active(true);
  _miFadeOutOption[_state->fadeOutSpeed].set_active(true);

  _toggleColumns.clear();
  _controls.clear();
  for (FaderState &state : _state->faders) {
    addControl(state.IsToggleButton(), state.NewToggleButtonColumn());
  }

  resize(_state->width, _state->height);

  for (size_t i = 0; i != _state->faders.size(); ++i)
    _controls[i]->Assign(_state->faders[i].GetSourceValue(), true);
}

void FaderWindow::updateValues() {
  /*boost::posix_time::ptime currentTime(
      boost::posix_time::microsec_clock::local_time());
  double timePassed =
      (double)(currentTime - _lastUpdateTime).total_microseconds() * 1e-6;
  _lastUpdateTime = std::move(currentTime);
   for (std::unique_ptr<ControlWidget> &cw : _controls) {
    cw->UpdateValue(timePassed);
  }*/
}

double FaderWindow::mapSliderToSpeed(int sliderVal) {
  switch (sliderVal) {
    default:
    case 0:
      return 0.0;
    case 1:
      return 10.0;
    case 2:
      return 5.0;
    case 3:
      return 3.5;
    case 4:
      return 2.0;
    case 5:
      return 1.0;
    case 6:
      return 0.5;
    case 7:
      return 0.33;
    case 8:
      return 0.25;
    case 9:
      return 0.16;
    case 10:
      return 0.1;
  }
}

std::string FaderWindow::speedLabel(int value) {
  switch (value) {
    default:
    case 0:
      return "Direct";
    case 1:
      return "0.1 (Fastest)";
    case 2:
      return "0.2s";
    case 3:
      return "0.3s";
    case 4:
      return "0.5s";
    case 5:
      return "1s";
    case 6:
      return "2s";
    case 7:
      return "3s";
    case 8:
      return "4s";
    case 9:
      return "6s";
    case 10:
      return "10s (Slowest)";
  }
}

void FaderWindow::onChangeDownSpeed() {
  _state->fadeOutSpeed = getFadeOutSpeed();
  double speed = mapSliderToSpeed(_state->fadeOutSpeed);

  for (std::unique_ptr<ControlWidget> &cw : _controls)
    cw->SetFadeDownSpeed(speed);
}

void FaderWindow::onChangeUpSpeed() {
  _state->fadeInSpeed = getFadeInSpeed();
  double speed = mapSliderToSpeed(_state->fadeInSpeed);

  for (std::unique_ptr<ControlWidget> &cw : _controls)
    cw->SetFadeUpSpeed(speed);
}

void FaderWindow::ChangeManagement(class Management &management,
                                   bool moveSliders) {
  _management = &management;
  for (std::unique_ptr<ControlWidget> &cw : _controls) {
    cw->ChangeManagement(management, moveSliders);
  }
}

size_t FaderWindow::getFadeInSpeed() const {
  for (size_t i = 0; i != 11; ++i)
    if (_miFadeInOption[i].get_active()) return i;
  return 0;
}

size_t FaderWindow::getFadeOutSpeed() const {
  for (size_t i = 0; i != 11; ++i)
    if (_miFadeOutOption[i].get_active()) return i;
  return 0;
}

void FaderWindow::ReloadValues() {
  for (std::unique_ptr<ControlWidget> &cw : _controls) {
    cw->MoveSlider();
  }
}
