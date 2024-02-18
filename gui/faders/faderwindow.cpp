#include "faderwindow.h"

#include "colorcontrolwidget.h"
#include "controlmenu.h"
#include "faderwidget.h"
#include "togglewidget.h"

#include "gui/eventtransmitter.h"
#include "gui/instance.h"

#include "gui/state/guistate.h"
#include "gui/dialogs/stringinputdialog.h"

#include "theatre/chase.h"
#include "theatre/dmxdevice.h"
#include "theatre/management.h"
#include "theatre/presetvalue.h"

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

FaderWindow::FaderWindow(size_t keyRowIndex) : _keyRowIndex(keyRowIndex) {
  initializeWidgets();
  initializeMenu();
}

FaderWindow::~FaderWindow() {
  _timeoutConnection.disconnect();
  if (_state) _state->isActive = false;
  Instance::State().EmitFaderSetChangeSignal();
}

void FaderWindow::LoadNew() {
  GUIState &state = Instance::State();
  state.FaderSets().emplace_back(std::make_unique<FaderSetState>());
  _state = state.FaderSets().back().get();
  _state->name = "Unnamed fader setup";
  for (size_t i = 0; i != 10; ++i)
    _state->faders.emplace_back(std::make_unique<FaderState>());

  _state->width = std::max(100, get_width());
  _state->height = std::max(300, get_height());
  _state->isActive = true;
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

    _crossFader.emplace(
        Gtk::Adjustment::create(
            0, 0, ControlValue::MaxUInt() + ControlValue::MaxUInt() / 100,
            (ControlValue::MaxUInt() + 1) / 100),
        Gtk::ORIENTATION_VERTICAL);
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

  set_title(_state->name);
  resize(_state->width, _state->height);

  for (size_t i = 0; i != _state->faders.size(); ++i) {
    _upperControls[i]->Assign(_state->faders[i]->GetSourceValues(), true);
    if (_miDualLayout.get_active())
      _lowerControls[i]->Assign(_state->faders[i]->GetSourceValues(), true);
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
  _menuButton.set_popup(_popupMenu);
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

  _miAddColorButton.signal_activate().connect(
      [&]() { onAddColorButtonClicked(); });
  _popupMenu.append(_miAddColorButton);

  _miAddToggleColumn.signal_activate().connect(
      [&]() { onAddToggleColumnClicked(); });
  _popupMenu.append(_miAddToggleColumn);

  _miRemoveFader.signal_activate().connect([&]() { onRemoveFaderClicked(); });
  _popupMenu.append(_miRemoveFader);

  _miRemove5Faders.signal_activate().connect(
      [&]() { onRemove5FadersClicked(); });
  _popupMenu.append(_miRemove5Faders);

  _miInputDevice.signal_activate().connect([&]() { onInputDeviceClicked(); });
  _popupMenu.append(_miInputDevice);

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
  addControlInLayout(_state->AddState(FaderControlType::Fader, false));
}

void FaderWindow::onAdd5FadersClicked() {
  for (size_t i = 0; i != 5; ++i)
    addControlInLayout(_state->AddState(FaderControlType::Fader, false));
}

void FaderWindow::onAddToggleClicked() {
  addControlInLayout(_state->AddState(FaderControlType::ToggleButton, false));
}

void FaderWindow::onAdd5ToggleControlsClicked() {
  for (size_t i = 0; i != 5; ++i)
    addControlInLayout(_state->AddState(FaderControlType::ToggleButton, false));
}

void FaderWindow::onAddColorButtonClicked() {
  addControlInLayout(_state->AddState(FaderControlType::ColorButton, false));
}

void FaderWindow::onAddToggleColumnClicked() {
  addControlInLayout(_state->AddState(FaderControlType::ToggleButton, true));
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
  switch (state.GetFaderType()) {
    case FaderControlType::Fader:
      control = std::make_unique<FaderWidget>(*this, state, controlMode, key);
      nameLabel = &static_cast<FaderWidget *>(control.get())->NameLabel();
      break;
    case FaderControlType::ToggleButton:
      control = std::make_unique<ToggleWidget>(*this, state, controlMode, key);
      nameLabel = nullptr;
      break;
    case FaderControlType::ColorButton:
      control =
          std::make_unique<ColorControlWidget>(*this, state, controlMode, key);
      nameLabel = nullptr;
      break;
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
  switch (state.GetFaderType()) {
    case FaderControlType::Fader:
      _controlGrid.attach(*control, hpos * 2 + 1, vpos, 2, 1);

      if (isUpper) {
        nameLabel->set_hexpand(true);
        const bool even = controls.size() % 2 == 0;
        _controlGrid.attach(*nameLabel, hpos * 2, even ? 1 : 2, 4, 1);
      }
      break;
    case FaderControlType::ToggleButton:
    case FaderControlType::ColorButton:
      if (newToggleColumn) {
        Gtk::VBox &new_box = column.emplace_back();
        _controlGrid.attach(new_box, hpos * 2 + 1, vpos, 2, 1);
        new_box.set_vexpand(false);
        new_box.set_valign(Gtk::ALIGN_START);
        column.back().show();
      }
      column.back().pack_start(*control);
      break;
  }

  control->show();
  controls.emplace_back(std::move(control));
}

void FaderWindow::removeFader() {
  FaderState &state = *_state->faders.back();
  const bool hasLower = _miDualLayout.get_active();
  if (state.GetFaderType() != FaderControlType::Fader &&
      state.NewToggleButtonColumn()) {
    _upperColumns.pop_back();
    if (hasLower) _lowerColumns.pop_back();
  }
  _upperControls.pop_back();
  if (hasLower) _lowerControls.pop_back();
  _state->faders.pop_back();
}

std::vector<size_t> FaderWindow::SingleSourceControls() const {
  std::vector<size_t> single_source_controls;
  for (size_t i = 0; i != _upperControls.size(); ++i) {
    if (_upperControls[i]->DefaultSourceCount() == 1)
      single_source_controls.emplace_back(i);
  }
  return single_source_controls;
}

void FaderWindow::onAssignClicked() {
  unassign();
  const bool hasLower = _miDualLayout.get_active();
  const std::vector<size_t> single_source_controls = SingleSourceControls();
  if (!single_source_controls.empty()) {
    std::vector<size_t>::const_iterator control_iter =
        single_source_controls.begin();
    const size_t n = Instance::Management().SourceValues().size();
    for (size_t i = 0; i != n; ++i) {
      theatre::SourceValue *source =
          Instance::Management().SourceValues()[i].get();
      if (!Instance::State().IsAssigned(source)) {
        _upperControls[*control_iter]->Assign({source}, true);
        if (hasLower) _lowerControls[*control_iter]->Assign({source}, true);
        ++control_iter;
        if (control_iter == single_source_controls.end()) break;
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
  const std::vector<size_t> single_source_controls = SingleSourceControls();
  if (!single_source_controls.empty()) {
    std::vector<size_t>::const_iterator control_iter =
        single_source_controls.begin();
    for (const std::unique_ptr<theatre::SourceValue> &sv :
         Instance::Management().SourceValues()) {
      theatre::Chase *c =
          dynamic_cast<theatre::Chase *>(&sv->GetControllable());
      if (c != nullptr) {
        _upperControls[*control_iter]->Assign({sv.get()}, true);
        if (hasLower) _lowerControls[*control_iter]->Assign({sv.get()}, true);
        ++control_iter;
        if (control_iter == single_source_controls.end()) break;
      }
    }
  }
}

void FaderWindow::onSoloToggled() {
  if (_recursionLock.IsFirst()) {
    _state->isSolo = _miSolo.get_active();
  }
}

void FaderWindow::onControlValueChanged(ControlWidget *widget) {
  if (_miSolo.get_active()) {
    // Limitting the controls might generate another control value change, but
    // since it is an auto generated change we will not apply the limit of that
    // change to other faders.
    if (_recursionLock.IsFirst()) {
      RecursionLock::Token token(_recursionLock);
      theatre::SourceValue *source = widget->GetSourceValue(0);
      unsigned new_value = source->A().Value().UInt();
      const double inverse = ControlWidget::MAX_SCALE_VALUE() - new_value -
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
    const std::vector<theatre::SourceValue *> &sources =
        _upperControls[widgetIndex]->GetSourceValues();
    _state->faders[widgetIndex]->SetSourceValues(sources);
    if (_miDualLayout.get_active()) {
      _lowerControls[widgetIndex]->Assign(sources, true);
    }
  }
}

bool FaderWindow::HandleKeyDown(char key) {
  if (_keyRowIndex >= 3) return false;

  for (unsigned i = 0; i < 10; ++i) {
    if (_keyRowsUpper[_keyRowIndex][i] == key) {
      if (i < _upperControls.size()) {
        _upperControls[i]->FlashOn();
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
        _upperControls[i]->FlashOff();
      }
      return true;
    }
  }
  return false;
}

bool FaderWindow::IsAssigned(theatre::SourceValue *source_value) const {
  for (const std::unique_ptr<ControlWidget> &c : _upperControls) {
    const std::vector<theatre::SourceValue *> &sources = c->GetSourceValues();
    if (std::find(sources.begin(), sources.end(), source_value) !=
        sources.end())
      return true;
  }
  return false;
}

void FaderWindow::onSetNameClicked() {
  Gtk::MessageDialog dialog(*this, "Name fader setup", false,
                            Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
  Gtk::Entry entry;
  dialog.get_message_area()->pack_start(entry, Gtk::PACK_SHRINK);
  dialog.get_message_area()->show_all_children();
  dialog.set_secondary_text("Please enter a name for this fader setup");
  int result = dialog.run();
  if (result == Gtk::RESPONSE_OK) {
    _state->name = entry.get_text();
    set_title(_state->name);
    Instance::State().EmitFaderSetChangeSignal();
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
  if (_connectedInputUniverse || _connectedMidiManager) {
    _connectedMidiManager->Update();  // TODO This should move to a higher level
    const size_t n = _upperControls.size();
    _inputValues.resize(n);
    _previousInputValues.resize(n);
    if (_connectedInputUniverse) {
      Instance::Management().Device()->GetInputValues(*_connectedInputUniverse,
                                                      _inputValues.data(), n);
    } else {
      for (size_t i = 0; i != std::min(n, _connectedMidiManager->GetNFaders());
           ++i) {
        unsigned char value = _connectedMidiManager->GetFaderValue(i);
        _inputValues[i] = std::min(value * 2, 255);
      }
    }
    size_t color_button_index = 0;
    for (size_t i = 0; i != n; ++i) {
      if (_upperControls[i]->GetSourceValues().size() == 1) {
        if (theatre::SourceValue *sv = _upperControls[i]->GetSourceValues()[0];
            _inputValues[i] != _previousInputValues[i] && sv) {
          sv->A().Set(ControlValue::CharToValue(_inputValues[i]));
        }
      } else if (ColorControlWidget *ccw = dynamic_cast<ColorControlWidget *>(
                     _upperControls[i].get());
                 ccw) {
        if (color_button_index < 2) {
          const std::optional<theatre::Color> color =
              _connectedMidiManager->GetColor(color_button_index);
          if (color) ccw->SetColor(*color);
          ++color_button_index;
        }
      }
    }
    std::swap(_previousInputValues, _inputValues);
  }

  theatre::SourceValue *assigned_source_value = nullptr;
  for (std::unique_ptr<ControlWidget> &cw : _upperControls) {
    cw->SyncFader();
    for (theatre::SourceValue *sv : cw->GetSourceValues()) {
      if (sv) {
        assigned_source_value = sv;
      }
    }
  }
  for (std::unique_ptr<ControlWidget> &cw : _lowerControls) {
    cw->SyncFader();
  }
  if (_crossFader && assigned_source_value) {
    const unsigned x_value = assigned_source_value->CrossFader().Value().UInt();
    if (_isCrossFaderStarted) {
      if (x_value == 0) {
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
      for (theatre::SourceValue *source : cw->GetSourceValues()) {
        if (source) {
          source->CrossFader().Set(_crossFader->get_value(), 0.0);
        }
      }
    }
  }
}

void FaderWindow::FlipCrossFader() {
  RecursionLock::Token token(_recursionLock);

  for (const std::unique_ptr<glight::gui::ControlWidget> &upper :
       _upperControls) {
    for (theatre::SourceValue *source : upper->GetSourceValues()) {
      if (source) {
        source->Swap();
      }
    }
  }
}

void FaderWindow::onStartCrossFader() {
  if (_crossFader->get_value() < ControlValue::MaxUInt() / 2) FlipCrossFader();
  for (std::unique_ptr<ControlWidget> &cw : _upperControls) {
    for (theatre::SourceValue *source : cw->GetSourceValues()) {
      if (source) {
        source->CrossFader().Set(0.0, MapSliderToSpeed(getFadeInSpeed()));
      }
    }
  }
  _isCrossFaderStarted = true;
}

void FaderWindow::AssignTopToBottom() {
  std::vector<std::unique_ptr<glight::gui::ControlWidget>> &controls =
      _lowerControls.empty() ? _upperControls : _lowerControls;
  for (size_t i = 0; i != controls.size(); ++i) {
    for (theatre::SourceValue *source : controls[i]->GetSourceValues()) {
      if (source) {
        source->B() = source->A();
        controls[i]->SyncFader();
      }
    }
  }
  _isCrossFaderStarted = false;
}

void FaderWindow::CrossFadeImmediately() {
  if (_crossFader->get_value() < ControlValue::MaxUInt() / 2) FlipCrossFader();
  for (std::unique_ptr<ControlWidget> &cw : _upperControls) {
    for (theatre::SourceValue *source : cw->GetSourceValues()) {
      if (source) {
        source->CrossFader().Set(0.0);
      }
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

void FaderWindow::onInputDeviceClicked() {
  _connectedInputUniverse.reset();
  StringInputDialog dialog(
      "Connect input universe to faders",
      "Enter universe number:\n(1 = first universe, leave empty to disconnect)",
      "1");
  if (dialog.run() == Gtk::RESPONSE_OK) {
    const size_t value = std::atoi(dialog.Value().c_str());
    if (value >= 1 && value <= Instance::Management().Device()->NUniverses())
      _connectedInputUniverse = value - 1;
  }
}

std::unique_ptr<ControlMenu> &FaderWindow::GetControlMenu() {
  return control_menu_;
}

}  // namespace glight::gui
