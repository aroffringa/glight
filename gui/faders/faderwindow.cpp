#include "faderwindow.h"
#include "faderwidget.h"
#include "togglewidget.h"

#include "../eventtransmitter.h"
#include "../guistate.h"

#include "../../theatre/chase.h"
#include "../../theatre/management.h"
#include "../../theatre/presetvalue.h"

#include <gtkmm/entry.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include <glibmm/main.h>

namespace glight::gui {

const char FaderWindow::_keyRowsUpper[3][10] = {
    {'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?'},
    {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':'},
    {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'}};
const char FaderWindow::_keyRowsLower[3][10] = {
    {'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'},
    {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';'},
    {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'}};

FaderWindow::FaderWindow(EventTransmitter &eventHub, GUIState &guiState,
                         theatre::Management &management, size_t keyRowIndex)
    : _management(&management),
      _keyRowIndex(keyRowIndex),
      _menuButton(),
      _miLayout("Layout"),
      _miFadeIn("Fade in"),
      _miFadeOut("Fade out"),
      _miName("Set name..."),
      _miSolo("Solo"),
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
      // Layout menu
      _miSimpleLayout("Simple"),
      _miDryModeLayout("Dry mode"),
      _miDualLayout("Dual"),
      _eventHub(eventHub),
      _guiState(guiState),
      _state(nullptr) {
  initializeWidgets();
  initializeMenu();
}

FaderWindow::~FaderWindow() {
  _timeoutConnection.disconnect();
  if (_state) _state->isActive = false;
  _guiState.EmitFaderSetupChangeSignal();
}

void FaderWindow::LoadNew() {
  _guiState.FaderSetups().emplace_back(std::make_unique<FaderSetupState>());
  _state = _guiState.FaderSetups().back().get();
  _state->name = "Unnamed fader setup";
  _state->isActive = true;
  for (size_t i = 0; i != 10; ++i) _state->faders.emplace_back();

  _state->width = std::max(100, get_width());
  _state->height = std::max(300, get_height());
  RecursionLock::Token token(_recursionLock);
  loadState();
}

void FaderWindow::LoadState(FaderSetupState *state) {
  _state = state;
  _state->isActive = true;
  RecursionLock::Token token(_recursionLock);
  loadState();
}

void FaderWindow::initializeWidgets() {
  set_title("Glight - faders");

  signal_configure_event().connect(sigc::mem_fun(*this, &FaderWindow::onResize),
                                   false);

  _timeoutConnection = Glib::signal_timeout().connect(
      sigc::mem_fun(*this, &FaderWindow::onTimeout), 40);

  add(_hBox);

  _menuButton.set_events(Gdk::BUTTON_PRESS_MASK);
  _menuButton.set_image_from_icon_name("document-properties");
  _menuButton.set_tooltip_text("Open options menu");
  _menuButton.set_menu(_popupMenu);
  _leftBox.pack_start(_menuButton, false, false, 5);
  _hBox.pack_start(_leftBox);

  _controlGrid.set_column_spacing(3);
  _hBox.pack_start(_controlGrid, true, true);

  show_all_children();
  set_default_size(0, 300);
}

void FaderWindow::initializeMenu() {
  Gtk::RadioMenuItem::Group layoutGroup;
  _miSimpleLayout.set_active(true);
  _miSimpleLayout.set_group(layoutGroup);
  _miSimpleLayout.signal_activate().connect([&]() { onLayoutChanged(); });
  _layoutMenu.append(_miSimpleLayout);
  _miDryModeLayout.set_group(layoutGroup);
  _miDryModeLayout.signal_activate().connect([&]() { onLayoutChanged(); });
  _layoutMenu.append(_miDryModeLayout);
  _miDualLayout.set_group(layoutGroup);
  _miDualLayout.signal_activate().connect([&]() { onLayoutChanged(); });
  _layoutMenu.append(_miDualLayout);
  _miLayout.set_submenu(_layoutMenu);
  _popupMenu.append(_miLayout);

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

  _miNameImage.set_from_icon_name("user-bookmarks",
                                  Gtk::BuiltinIconSize::ICON_SIZE_MENU);
  _miName.set_image(_miNameImage);
  _miName.signal_activate().connect([&]() { onSetNameClicked(); });
  _popupMenu.append(_miName);

  _miSolo.signal_activate().connect([&]() { onSoloToggled(); });
  _popupMenu.append(_miSolo);

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

void FaderWindow::addControl(bool isToggle, bool newToggleColumn,
                             bool isPrimary) {
  std::vector<std::unique_ptr<ControlWidget>> &controls =
      isPrimary ? _controlsA : _controlsB;
  std::vector<Gtk::VBox> &column =
      isPrimary ? _toggleColumnsA : _toggleColumnsB;
  if (column.empty()) newToggleColumn = true;
  if (_recursionLock.IsFirst()) {
    _state->faders.emplace_back();
    FaderState &state = _state->faders.back();
    state.SetIsToggleButton(isToggle);
    state.SetNewToggleButtonColumn(newToggleColumn);
  }
  bool hasKey = _controlsA.size() < 10 && _keyRowIndex < 3 && isPrimary;
  char key = hasKey ? _keyRowsLower[_keyRowIndex][_controlsA.size()] : ' ';

  Gtk::Widget *nameLabel;
  std::unique_ptr<ControlWidget> control;
  const ControlMode controlMode =
      isPrimary ? ControlMode::Primary : ControlMode::Secondary;
  if (isToggle) {
    control = std::make_unique<ToggleWidget>(*_management, _eventHub,
                                             controlMode, key);
    nameLabel = nullptr;
  } else {
    control = std::make_unique<FaderWidget>(*_management, _eventHub,
                                            controlMode, key);
    nameLabel = &static_cast<FaderWidget *>(control.get())->NameLabel();
  }

  control->SetFadeDownSpeed(mapSliderToSpeed(getFadeOutSpeed()));
  control->SetFadeUpSpeed(mapSliderToSpeed(getFadeInSpeed()));
  const size_t controlIndex = controls.size();
  control->SignalValueChange().connect(
      sigc::bind(sigc::mem_fun(*this, &FaderWindow::onControlValueChanged),
                 control.get()));
  control->SignalAssigned().connect(sigc::bind(
      sigc::mem_fun(*this, &FaderWindow::onControlAssigned), controlIndex));

  const size_t vpos = isPrimary ? 0 : 3;
  const size_t hpos = controls.size() + column.size();
  if (isToggle) {
    if (newToggleColumn) {
      _controlGrid.attach(column.emplace_back(), hpos * 2 + 1, vpos, 2, 1);
      column.back().show();
    }
    column.back().pack_start(*control);
  } else {
    _controlGrid.attach(*control, hpos * 2 + 1, vpos, 2, 1);

    if (isPrimary) {
      nameLabel->set_hexpand(true);
      const bool even = controls.size() % 2 == 0;
      _controlGrid.attach(*nameLabel, hpos * 2, even ? 1 : 2, 4, 1);
    }
  }

  control->show();
  controls.emplace_back(std::move(control));
}

void FaderWindow::removeFader() {
  FaderState &state = _state->faders.back();
  if (state.IsToggleButton() && state.NewToggleButtonColumn())
    _toggleColumnsA.pop_back();
  _controlsA.pop_back();
  _state->faders.pop_back();
}

void FaderWindow::onAssignClicked() {
  for (std::unique_ptr<ControlWidget> &c : _controlsA) c->Unassign();
  size_t n = _management->SourceValues().size();
  if (!_controlsA.empty()) {
    size_t controlIndex = 0;
    for (size_t i = 0; i != n; ++i) {
      theatre::SourceValue *sv = _management->SourceValues()[i].get();
      if (!_guiState.IsAssigned(sv)) {
        _controlsA[controlIndex]->Assign(sv, true);
        ++controlIndex;
        if (controlIndex == _controlsA.size()) break;
      }
    }
  }
}

void FaderWindow::onClearClicked() {
  for (std::unique_ptr<ControlWidget> &c : _controlsA) c->Unassign();
}

void FaderWindow::onAssignChasesClicked() {
  if (!_controlsA.empty()) {
    size_t controlIndex = 0;
    for (size_t i = 0; i != _management->SourceValues().size(); ++i) {
      theatre::SourceValue *sv = _management->SourceValues()[i].get();
      theatre::Chase *c =
          dynamic_cast<theatre::Chase *>(&sv->GetControllable());
      if (c != nullptr) {
        _controlsA[controlIndex]->Assign(sv, true);
        ++controlIndex;
        if (controlIndex == _controlsA.size()) break;
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
      for (std::unique_ptr<ControlWidget> &c : _controlsA) {
        if (c.get() != widget) c->Limit(limitValue);
      }
    }
  }
}

void FaderWindow::onControlAssigned(size_t widgetIndex) {
  if (_recursionLock.IsFirst())
    _state->faders[widgetIndex].SetSourceValue(
        _controlsA[widgetIndex]->GetSourceValue());
}

bool FaderWindow::HandleKeyDown(char key) {
  if (_keyRowIndex >= 3) return false;

  for (unsigned i = 0; i < 10; ++i) {
    if (_keyRowsUpper[_keyRowIndex][i] == key) {
      if (i < _controlsA.size()) {
        _controlsA[i]->FullOn();
      }
      return true;
    } else if ((_keyRowsLower[_keyRowIndex][i]) == key) {
      if (i < _controlsA.size()) {
        _controlsA[i]->Toggle();
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
      if (i < _controlsA.size()) {
        _controlsA[i]->FullOff();
      }
      return true;
    }
  }
  return false;
}

bool FaderWindow::IsAssigned(theatre::SourceValue *sourceValue) {
  for (std::unique_ptr<ControlWidget> &c : _controlsA) {
    if (c->GetSourceValue() == sourceValue) return true;
  }
  return false;
}

void FaderWindow::onSetNameClicked() {
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

void FaderWindow::loadState() {
  _miSolo.set_active(_state->isSolo);
  _miFadeInOption[_state->fadeInSpeed].set_active(true);
  _miFadeOutOption[_state->fadeOutSpeed].set_active(true);

  _toggleColumnsA.clear();
  _toggleColumnsB.clear();
  _controlsA.clear();
  _controlsB.clear();
  for (FaderState &state : _state->faders) {
    addControlInLayout(state.IsToggleButton(), state.NewToggleButtonColumn());
  }

  resize(_state->width, _state->height);

  for (size_t i = 0; i != _state->faders.size(); ++i) {
    _controlsA[i]->Assign(_state->faders[i].GetSourceValue(), true);
    if (_miDualLayout.get_active())
      _controlsB[i]->Assign(_state->faders[i].GetSourceValue(), true);
  }
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

  for (std::unique_ptr<ControlWidget> &cw : _controlsA)
    cw->SetFadeDownSpeed(speed);
}

void FaderWindow::onChangeUpSpeed() {
  _state->fadeInSpeed = getFadeInSpeed();
  double speed = mapSliderToSpeed(_state->fadeInSpeed);

  for (std::unique_ptr<ControlWidget> &cw : _controlsA)
    cw->SetFadeUpSpeed(speed);
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
  for (std::unique_ptr<ControlWidget> &cw : _controlsA) {
    cw->MoveSlider();
  }
}

}  // namespace glight::gui
