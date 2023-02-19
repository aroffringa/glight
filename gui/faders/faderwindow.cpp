#include "faderwindow.h"

#include "controlmenu.h"
#include "faderwidget.h"
#include "togglewidget.h"

#include "../eventtransmitter.h"
#include "../state/guistate.h"

#include "../../theatre/chase.h"
#include "../../theatre/management.h"
#include "../../theatre/presetvalue.h"

#include <gtkmm/entry.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include <glibmm/main.h>

namespace glight::gui {

using theatre::ControlValue;

namespace {
double MapSliderToSpeed(int sliderVal) {
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

std::string SpeedLabel(int value) {
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

}  // namespace

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
    : _management(management),
      _keyRowIndex(keyRowIndex),

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
      _miPrimaryLayout("Primary"),
      _miSecondaryLayout("Secondary"),
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
  _guiState.EmitFaderSetChangeSignal();
}

void FaderWindow::LoadNew() {
  _guiState.FaderSets().emplace_back(std::make_unique<FaderSetState>());
  _state = _guiState.FaderSets().back().get();
  _state->name = "Unnamed fader setup";
  for (size_t i = 0; i != 10; ++i)
    _state->faders.emplace_back(std::make_unique<FaderState>());

  _state->width = std::max(100, get_width());
  _state->height = std::max(300, get_height());
  RecursionLock::Token token(_recursionLock);
  loadState();
}

void FaderWindow::LoadState(FaderSetState *state) {
  _state = state;
  _state->isActive = true;
  RecursionLock::Token token(_recursionLock);
  loadState();
}

void FaderWindow::loadState() {
  _miSolo.set_active(_state->isSolo);
  switch (_state->mode) {
    case FaderSetMode::Primary:
      _miPrimaryLayout.set_active(true);
      break;
    case FaderSetMode::Secondary:
      _miSecondaryLayout.set_active(true);
      break;
    case FaderSetMode::Dual:
      _miDualLayout.set_active(true);
      break;
  }
  _miFadeInOption[_state->fadeInSpeed].set_active(true);
  _miFadeOutOption[_state->fadeOutSpeed].set_active(true);
  _crossFader.reset();
  _immediateCrossFadeButton.reset();
  _activateCrossFaderButton.reset();
  _upperColumns.clear();
  _lowerColumns.clear();
  _upperControls.clear();
  _lowerControls.clear();

  // Show master cross fader?
  const bool show_master_fader =
      _miDualLayout.get_active() || _miSecondaryLayout.get_active();
  if (show_master_fader) {
    _activateCrossFaderButton.emplace();
    _activateCrossFaderButton->set_image_from_icon_name("media-playback-start");
    _activateCrossFaderButton->set_tooltip_text(
        "Activate fading of the cross-fader to smoothly make the secondary "
        "setting the primary setting");
    _activateCrossFaderButton->signal_clicked().connect(
        [&]() { onStartCrossFader(); });
    _leftBox.pack_start(*_activateCrossFaderButton, false, false);
    _activateCrossFaderButton->show();

    _immediateCrossFadeButton.emplace();
    _immediateCrossFadeButton->set_image_from_icon_name("go-jump");
    _immediateCrossFadeButton->set_tooltip_text(
        "Directly switch the cross-fade");
    _immediateCrossFadeButton->signal_clicked().connect(
        [&]() { CrossFadeImmediately(); });
    _leftBox.pack_start(*_immediateCrossFadeButton, false, false);
    _immediateCrossFadeButton->show();

    _crossFader.emplace(0,
                        ControlValue::MaxUInt() + ControlValue::MaxUInt() / 100,
                        (ControlValue::MaxUInt() + 1) / 100);
    _leftBox.pack_start(*_crossFader, true, true);
    _crossFader->set_value(0);
    _crossFader->set_draw_value(false);
    _crossFader->set_has_origin(false);
    _crossFader->set_vexpand(true);
    _crossFader->signal_value_changed().connect(
        sigc::mem_fun(*this, &FaderWindow::onCrossFaderChange));
    _crossFader->show();
  }
  for (std::unique_ptr<FaderState> &state : _state->faders) {
    addControlInLayout(*state);
  }

  resize(_state->width, _state->height);

  for (size_t i = 0; i != _state->faders.size(); ++i) {
    _upperControls[i]->Assign(_state->faders[i]->GetSourceValue(), true);
    if (_miDualLayout.get_active())
      _lowerControls[i]->Assign(_state->faders[i]->GetSourceValue(), true);
  }
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
  _hBox.pack_start(_leftBox, false, false);

  _controlGrid.set_column_spacing(3);
  _hBox.pack_start(_controlGrid, true, true);

  show_all_children();
  set_default_size(0, 300);
}

void FaderWindow::initializeMenu() {
  Gtk::RadioMenuItem::Group layoutGroup;
  _miPrimaryLayout.set_active(true);
  _miPrimaryLayout.set_group(layoutGroup);
  _miPrimaryLayout.signal_activate().connect([&]() { onLayoutChanged(); });
  _layoutMenu.append(_miPrimaryLayout);
  _miSecondaryLayout.set_group(layoutGroup);
  _miSecondaryLayout.signal_activate().connect([&]() { onLayoutChanged(); });
  _layoutMenu.append(_miSecondaryLayout);
  _miDualLayout.set_group(layoutGroup);
  _miDualLayout.signal_activate().connect([&]() { onLayoutChanged(); });
  _layoutMenu.append(_miDualLayout);
  _miLayout.set_submenu(_layoutMenu);
  _popupMenu.append(_miLayout);

  Gtk::RadioMenuItem::Group fadeInGroup;
  Gtk::RadioMenuItem::Group fadeOutGroup;
  for (size_t i = 0; i != 11; ++i) {
    _miFadeInOption[i].set_label(SpeedLabel(i));
    _miFadeInOption[i].set_group(fadeInGroup);
    _miFadeInOption[i].signal_activate().connect([&]() { onChangeUpSpeed(); });
    _fadeInMenu.append(_miFadeInOption[i]);

    _miFadeOutOption[i].set_label(SpeedLabel(i));
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

  _miClear.signal_activate().connect([&]() { unassign(); });
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

bool FaderWindow::onResize(GdkEventConfigure * /*event*/) {
  if (_recursionLock.IsFirst()) {
    _state->height = get_height();
    _state->width = get_width();
  }
  return false;
}

void FaderWindow::onAddFaderClicked() {
  addControlInLayout(_state->AddState(false, false));
}

void FaderWindow::onAdd5FadersClicked() {
  for (size_t i = 0; i != 5; ++i)
    addControlInLayout(_state->AddState(false, false));
}

void FaderWindow::onAdd5ToggleControlsClicked() {
  for (size_t i = 0; i != 5; ++i)
    addControlInLayout(_state->AddState(true, false));
}

void FaderWindow::onAddToggleClicked() {
  addControlInLayout(_state->AddState(true, false));
}

void FaderWindow::onAddToggleColumnClicked() {
  addControlInLayout(_state->AddState(true, true));
}

void FaderWindow::addControl(FaderState &state, bool isUpper) {
  std::vector<std::unique_ptr<ControlWidget>> &controls =
      isUpper ? _upperControls : _lowerControls;
  std::vector<Gtk::VBox> &column = isUpper ? _upperColumns : _lowerColumns;
  bool newToggleColumn = state.NewToggleButtonColumn() || column.empty();
  const bool hasKey = _upperControls.size() < 10 && _keyRowIndex < 3 && isUpper;
  const char key =
      hasKey ? _keyRowsLower[_keyRowIndex][_upperControls.size()] : ' ';

  Gtk::Widget *nameLabel = nullptr;
  std::unique_ptr<ControlWidget> control;
  const bool isSecondary = !isUpper || _miSecondaryLayout.get_active();
  const ControlMode controlMode =
      isSecondary ? ControlMode::Secondary : ControlMode::Primary;
  if (state.IsToggleButton()) {
    control = std::make_unique<ToggleWidget>(*this, state, controlMode, key);
    nameLabel = nullptr;
  } else {
    control = std::make_unique<FaderWidget>(*this, state, controlMode, key);
    nameLabel = &static_cast<FaderWidget *>(control.get())->NameLabel();
  }

  control->SetFadeDownSpeed(MapSliderToSpeed(getFadeOutSpeed()));
  control->SetFadeUpSpeed(MapSliderToSpeed(getFadeInSpeed()));
  const size_t controlIndex = controls.size();
  control->SignalValueChange().connect(
      sigc::bind(sigc::mem_fun(*this, &FaderWindow::onControlValueChanged),
                 control.get()));
  if (isUpper)
    control->SignalAssigned().connect(sigc::bind(
        sigc::mem_fun(*this, &FaderWindow::onControlAssigned), controlIndex));

  const size_t vpos = isUpper ? 0 : 3;
  const size_t hpos = controls.size() + column.size();
  if (state.IsToggleButton()) {
    if (newToggleColumn) {
      _controlGrid.attach(column.emplace_back(), hpos * 2 + 1, vpos, 2, 1);
      column.back().show();
    }
    column.back().pack_start(*control);
  } else {
    _controlGrid.attach(*control, hpos * 2 + 1, vpos, 2, 1);

    if (isUpper) {
      nameLabel->set_hexpand(true);
      const bool even = controls.size() % 2 == 0;
      _controlGrid.attach(*nameLabel, hpos * 2, even ? 1 : 2, 4, 1);
    }
  }

  control->show();
  controls.emplace_back(std::move(control));
}

void FaderWindow::removeFader() {
  FaderState &state = *_state->faders.back();
  const bool hasLower = _miDualLayout.get_active();
  if (state.IsToggleButton() && state.NewToggleButtonColumn()) {
    _upperColumns.pop_back();
    if (hasLower) _lowerColumns.pop_back();
  }
  _upperControls.pop_back();
  if (hasLower) _lowerControls.pop_back();
  _state->faders.pop_back();
}

void FaderWindow::onAssignClicked() {
  unassign();
  const bool hasLower = _miDualLayout.get_active();
  if (!_upperControls.empty()) {
    size_t controlIndex = 0;
    const size_t n = _management.SourceValues().size();
    for (size_t i = 0; i != n; ++i) {
      theatre::SourceValue *source = _management.SourceValues()[i].get();
      if (!_guiState.IsAssigned(source)) {
        _upperControls[controlIndex]->Assign(source, true);
        if (hasLower) _lowerControls[controlIndex]->Assign(source, true);
        ++controlIndex;
        if (controlIndex == _upperControls.size()) break;
      }
    }
  }
}

void FaderWindow::unassign() {
  for (std::unique_ptr<ControlWidget> &c : _lowerControls) c->Unassign();
  for (std::unique_ptr<ControlWidget> &c : _upperControls) c->Unassign();
}

void FaderWindow::onAssignChasesClicked() {
  unassign();
  const bool hasLower = _miDualLayout.get_active();
  if (!_upperControls.empty()) {
    size_t controlIndex = 0;
    for (const std::unique_ptr<theatre::SourceValue> &sv :
         _management.SourceValues()) {
      theatre::Chase *c =
          dynamic_cast<theatre::Chase *>(&sv->GetControllable());
      if (c != nullptr) {
        _upperControls[controlIndex]->Assign(sv.get(), true);
        if (hasLower) _lowerControls[controlIndex]->Assign(sv.get(), true);
        ++controlIndex;
        if (controlIndex == _upperControls.size()) break;
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
      const double inverse = ControlWidget::MAX_SCALE_VALUE() - newValue -
                             ControlWidget::MAX_SCALE_VALUE() * 0.01;
      const double limitValue = std::max(0.0, inverse);
      const bool isLower = widget->GetMode() == ControlMode::Secondary &&
                           _miDualLayout.get_active();
      std::vector<std::unique_ptr<glight::gui::ControlWidget>> &controls =
          isLower ? _lowerControls : _upperControls;
      for (std::unique_ptr<ControlWidget> &c : controls) {
        if (c.get() != widget) c->Limit(limitValue);
      }
    }
  }
}

void FaderWindow::onControlAssigned(size_t widgetIndex) {
  if (_recursionLock.IsFirst()) {
    theatre::SourceValue *source =
        _upperControls[widgetIndex]->GetSourceValue();
    _state->faders[widgetIndex]->SetSourceValue(source);
    if (_miDualLayout.get_active()) {
      _lowerControls[widgetIndex]->Assign(source, true);
    }
  }
}

bool FaderWindow::HandleKeyDown(char key) {
  if (_keyRowIndex >= 3) return false;

  for (unsigned i = 0; i < 10; ++i) {
    if (_keyRowsUpper[_keyRowIndex][i] == key) {
      if (i < _upperControls.size()) {
        _upperControls[i]->FullOn();
      }
      return true;
    } else if ((_keyRowsLower[_keyRowIndex][i]) == key) {
      if (i < _upperControls.size()) {
        _upperControls[i]->Toggle();
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
      if (i < _upperControls.size()) {
        _upperControls[i]->FullOff();
      }
      return true;
    }
  }
  return false;
}

bool FaderWindow::IsAssigned(theatre::SourceValue *sourceValue) const {
  for (const std::unique_ptr<ControlWidget> &c : _upperControls) {
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
    _guiState.EmitFaderSetChangeSignal();
  }
}

void FaderWindow::onChangeDownSpeed() {
  _state->fadeOutSpeed = getFadeOutSpeed();
  const double speed = MapSliderToSpeed(_state->fadeOutSpeed);

  for (std::unique_ptr<ControlWidget> &cw : _upperControls)
    cw->SetFadeDownSpeed(speed);
}

void FaderWindow::onChangeUpSpeed() {
  _state->fadeInSpeed = getFadeInSpeed();
  const double speed = MapSliderToSpeed(_state->fadeInSpeed);

  for (std::unique_ptr<ControlWidget> &cw : _upperControls)
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

void FaderWindow::UpdateValues() {
  theatre::SourceValue *assigned_source_value = nullptr;
  for (std::unique_ptr<ControlWidget> &cw : _upperControls) {
    cw->MoveSlider();
    if (theatre::SourceValue *sv = cw->GetSourceValue(); sv) {
      assigned_source_value = sv;
    }
  }
  for (std::unique_ptr<ControlWidget> &cw : _lowerControls) {
    cw->MoveSlider();
  }
  if (_crossFader && assigned_source_value) {
    const unsigned x_value = assigned_source_value->CrossFader().Value().UInt();
    if (_isCrossFaderStarted) {
      if (x_value == 0.0) {
        AssignTopToBottom();
      }
    }
    RecursionLock::Token token(_recursionLock);
    _crossFader->set_value(x_value);
  }
}

void FaderWindow::onCrossFaderChange() {
  if (_recursionLock.IsFirst()) {
    for (std::unique_ptr<ControlWidget> &cw : _upperControls) {
      glight::theatre::SourceValue *source = cw->GetSourceValue();
      if (source) {
        source->CrossFader().Set(_crossFader->get_value(), 0.0);
      }
    }
  }
}

void FaderWindow::FlipCrossFader() {
  RecursionLock::Token token(_recursionLock);

  for (const std::unique_ptr<glight::gui::ControlWidget> &upper :
       _upperControls) {
    theatre::SourceValue *source = upper->GetSourceValue();
    if (source) {
      source->Swap();
    }
  }
}

void FaderWindow::onStartCrossFader() {
  if (_crossFader->get_value() < ControlValue::MaxUInt() / 2) FlipCrossFader();
  for (std::unique_ptr<ControlWidget> &cw : _upperControls) {
    glight::theatre::SourceValue *source = cw->GetSourceValue();
    if (source) {
      source->CrossFader().Set(0.0, MapSliderToSpeed(getFadeInSpeed()));
    }
  }
  _isCrossFaderStarted = true;
}

void FaderWindow::AssignTopToBottom() {
  std::vector<std::unique_ptr<glight::gui::ControlWidget>> &controls =
      _lowerControls.empty() ? _upperControls : _lowerControls;
  for (size_t i = 0; i != controls.size(); ++i) {
    glight::theatre::SourceValue *value = controls[i]->GetSourceValue();
    if (value) {
      value->B() = value->A();
      controls[i]->MoveSlider();
    }
  }
  _isCrossFaderStarted = false;
}

void FaderWindow::CrossFadeImmediately() {
  if (_crossFader->get_value() < ControlValue::MaxUInt() / 2) FlipCrossFader();
  for (std::unique_ptr<ControlWidget> &cw : _upperControls) {
    glight::theatre::SourceValue *source = cw->GetSourceValue();
    if (source) {
      source->CrossFader().Set(0.0);
    }
  }
  AssignTopToBottom();
}

void FaderWindow::onLayoutChanged() {
  if (_recursionLock.IsFirst()) {
    RecursionLock::Token token(_recursionLock);
    if (_miPrimaryLayout.get_active())
      _state->mode = FaderSetMode::Primary;
    else if (_miSecondaryLayout.get_active())
      _state->mode = FaderSetMode::Secondary;
    else  // if(_miDualLayout.get_active())
      _state->mode = FaderSetMode::Dual;
    loadState();
  }
}

std::unique_ptr<ControlMenu> &FaderWindow::GetControlMenu() {
  return control_menu_;
}

}  // namespace glight::gui
