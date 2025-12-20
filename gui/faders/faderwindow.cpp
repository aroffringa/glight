#include "faderwindow.h"

#include "colorcontrolwidget.h"
#include "combocontrolwidget.h"
#include "controlmenu.h"
#include "faderwidget.h"
#include "moverwidget.h"
#include "togglewidget.h"

#include "gui/eventtransmitter.h"
#include "gui/instance.h"
#include "gui/menufunctions.h"

#include "gui/dialogs/stringinputdialog.h"

#include "theatre/chase.h"
#include "theatre/management.h"
#include "theatre/presetvalue.h"

#include "uistate/uistate.h"

#include <gtkmm/entry.h>
#include <gtkmm/messagedialog.h>

#include <glibmm/main.h>

namespace glight::gui {

using theatre::ControlValue;
using uistate::FaderControlType;
using uistate::FaderSetMode;
using uistate::FaderSetState;
using uistate::FaderState;
using uistate::UIState;

namespace {
constexpr double MapSliderToSpeed(int sliderVal) {
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
  SaveSize();
  _timeoutConnection.disconnect();
  if (_state) _state->isActive = false;
  Instance::State().EmitFaderSetChangeSignal();
}

void FaderWindow::LoadNew() {
  UIState &state = Instance::State();
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
  solo_action_->set_state(Glib::Variant<bool>::create(_state->isSolo));
  switch (_state->mode) {
    case FaderSetMode::Primary:
      layout_action_->set_state(
          Glib::Variant<Glib::ustring>::create("primary"));
      break;
    case FaderSetMode::Secondary:
      layout_action_->set_state(
          Glib::Variant<Glib::ustring>::create("secondary"));
      break;
    case FaderSetMode::Dual:
      layout_action_->set_state(Glib::Variant<Glib::ustring>::create("dual"));
      break;
  }
  fade_in_action_->set_state(Glib::Variant<int>::create(_state->fadeInSpeed));
  fade_out_action_->set_state(Glib::Variant<int>::create(_state->fadeOutSpeed));
  _crossFader.reset();
  _immediateCrossFadeButton.reset();
  _activateCrossFaderButton.reset();
  _upperColumns.clear();
  _lowerColumns.clear();
  _upperControls.clear();
  _lowerControls.clear();

  // Show master cross fader?
  const bool show_master_fader = GetLayout() != "primary";
  if (show_master_fader) {
    _activateCrossFaderButton.emplace();
    _activateCrossFaderButton->set_image_from_icon_name("media-playback-start");
    _activateCrossFaderButton->set_tooltip_text(
        "Activate fading of the cross-fader to smoothly make the secondary "
        "setting the primary setting");
    _activateCrossFaderButton->signal_clicked().connect(
        [&]() { onStartCrossFader(); });
    _leftBox.append(*_activateCrossFaderButton);
    _activateCrossFaderButton->show();

    _immediateCrossFadeButton.emplace();
    _immediateCrossFadeButton->set_image_from_icon_name("go-jump");
    _immediateCrossFadeButton->set_tooltip_text(
        "Directly switch the cross-fade");
    _immediateCrossFadeButton->signal_clicked().connect(
        [&]() { CrossFadeImmediately(); });
    _leftBox.append(*_immediateCrossFadeButton);
    _immediateCrossFadeButton->show();

    _crossFader.emplace(
        Gtk::Adjustment::create(
            0, 0, ControlValue::MaxUInt() + ControlValue::MaxUInt() / 100,
            (ControlValue::MaxUInt() + 1) / 100),
        Gtk::Orientation::VERTICAL);
    _leftBox.append(*_crossFader);
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
  set_default_size(_state->width, _state->height);

  for (size_t i = 0; i != _state->faders.size(); ++i) {
    _upperControls[i]->Assign(_state->faders[i]->GetSourceValues(), true);
    if (GetLayout() == "dual")
      _lowerControls[i]->Assign(_state->faders[i]->GetSourceValues(), true);
  }
}

void FaderWindow::initializeWidgets() {
  set_title("Glight - faders");

  _timeoutConnection = Glib::signal_timeout().connect(
      sigc::mem_fun(*this, &FaderWindow::onTimeout), 40);

  set_child(_hBox);

  _hBox.append(_leftBox);

  _controlGrid.set_column_spacing(3);
  _hBox.append(_controlGrid);

  set_default_size(0, 300);
}

void FaderWindow::initializeMenu() {
  auto actions = Gio::SimpleActionGroup::create();
  const auto Add = [&actions](std::shared_ptr<Gio::Menu> &menu,
                              const Glib::ustring &label,
                              const Glib::ustring &action_name,
                              const sigc::slot<void()> &slot) {
    return AddMenuItem(*actions, menu, label, action_name, slot);
  };
  const auto Toggle =
      [&actions](std::shared_ptr<Gio::Menu> &menu, const Glib::ustring &label,
                 const Glib::ustring &action_name, bool initial_value,
                 const sigc::slot<void(bool)> &slot) {
        return AddToggleMenuItem(*actions, menu, label, action_name,
                                 initial_value, slot);
      };

  auto menu = Gio::Menu::create();
  auto submenu_section = Gio::Menu::create();

  auto layout_menu = Gio::Menu::create();
  layout_action_ = Gio::SimpleAction::create_radio_string("layout", "primary");
  layout_action_->signal_change_state().connect(
      [&](const Glib::VariantBase &new_value) {
        onLayoutChanged(
            static_cast<const Glib::Variant<Glib::ustring> &>(new_value).get());
      });
  actions->add_action(layout_action_);
  layout_menu->append("Primary", "win.layout::primary");
  layout_menu->append("Secondary", "win.layout::secondary");
  layout_menu->append("Dual", "win.layout::dual");
  submenu_section->append_submenu("Layout", layout_menu);

  auto fade_in = Gio::Menu::create();
  auto fade_out = Gio::Menu::create();

  fade_in_action_ = Gio::SimpleAction::create_radio_integer("fade_in", 0);
  fade_in_action_->signal_change_state().connect(
      [&](const Glib::VariantBase &) { onChangeUpSpeed(); });
  actions->add_action(fade_in_action_);

  fade_out_action_ = Gio::SimpleAction::create_radio_integer("fade_out", 0);
  fade_out_action_->signal_change_state().connect(
      [&](const Glib::VariantBase &) { onChangeDownSpeed(); });
  actions->add_action(fade_out_action_);

  for (size_t i = 0; i != 11; ++i) {
    fade_in->append(SpeedLabel(i), "fade_in(" + std::to_string(i) + ")");
    fade_out->append(SpeedLabel(i), "fade_out(" + std::to_string(i) + ")");
  }
  submenu_section->append_submenu("Fade in", fade_in);
  submenu_section->append_submenu("Fade out", fade_out);
  menu->append_section(submenu_section);

  auto options_section = Gio::Menu::create();
  solo_action_ = Toggle(options_section, "Solo", "solo", false,
                        [&](bool new_value) { onSoloToggled(new_value); });
  Add(options_section, "Assign", "assign", [&]() { onAssignClicked(); });
  Add(options_section, "Assign to chases", "assign_chases",
      [&]() { onAssignChasesClicked(); });
  Add(options_section, "Clear", "clear", [&]() { unassign(); });
  Add(options_section, "Set name...", "set_name",
      [&]() { onSetNameClicked(); });
  menu->append_section(options_section);

  auto controls_section = Gio::Menu::create();
  Add(controls_section, "Add fader", "add_1_fader",
      [&]() { onAddFaderClicked(); });
  Add(controls_section, "Add 5 faders", "add_5_faders",
      [&]() { onAdd5FadersClicked(); });
  Add(controls_section, "Add toggle control", "add_1_toggle",
      [&]() { onAddToggleClicked(); });
  Add(controls_section, "Add 5 toggle controls", "add_5_toggle",
      [&]() { onAdd5ToggleControlsClicked(); });
  Add(controls_section, "Add color button", "add_color",
      [&]() { onAddColorButtonClicked(); });
  Add(controls_section, "Add combo button", "add_combo",
      [&]() { onAddComboButtonClicked(); });
  Add(controls_section, "Add mover control", "add_mover",
      [&]() { onAddMoverButtonClicked(); });
  Add(controls_section, "Add toggle column", "add_toggle",
      [&]() { onAddToggleColumnClicked(); });
  Add(controls_section, "Remove 1", "remove_1",
      [&]() { onRemoveFaderClicked(); });
  Add(controls_section, "Remove 5", "remove_5",
      [&]() { onRemove5FadersClicked(); });
  Add(controls_section, "Input device...", "set_input_device",
      [&]() { onInputDeviceClicked(); });
  menu->append_section(controls_section);

  menu_button_.set_icon_name("open-menu-symbolic");
  menu_button_.set_menu_model(menu);
  menu_button_.set_tooltip_text("Control options menu");
  menu_button_.set_has_frame(false);
  insert_action_group("win", actions);

  header_bar_.pack_end(menu_button_);
  set_titlebar(header_bar_);
}

void FaderWindow::SaveSize() {
  if (_recursionLock.IsFirst()) {
    _state->height = get_height();
    _state->width = get_width();
  }
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

void FaderWindow::onAddComboButtonClicked() {
  addControlInLayout(_state->AddState(FaderControlType::ComboButton, false));
}

void FaderWindow::onAddMoverButtonClicked() {
  addControlInLayout(_state->AddState(FaderControlType::MoverControl, false));
}

void FaderWindow::onAddToggleColumnClicked() {
  addControlInLayout(_state->AddState(FaderControlType::ToggleButton, true));
}

void FaderWindow::addControl(FaderState &state, bool isUpper) {
  std::vector<std::unique_ptr<ControlWidget>> &controls =
      isUpper ? _upperControls : _lowerControls;
  std::vector<Gtk::Box> &column = isUpper ? _upperColumns : _lowerColumns;
  bool newToggleColumn = state.NewToggleButtonColumn() || column.empty();
  const bool hasKey = _upperControls.size() < 10 && _keyRowIndex < 3 && isUpper;
  const char key =
      hasKey ? _keyRowsLower[_keyRowIndex][_upperControls.size()] : ' ';

  Gtk::Widget *nameLabel = nullptr;
  std::unique_ptr<ControlWidget> control;
  const bool isSecondary = !isUpper || GetLayout() == "secondary";
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
    case FaderControlType::ComboButton:
      control =
          std::make_unique<ComboControlWidget>(*this, state, controlMode, key);
      nameLabel = nullptr;
      break;
    case FaderControlType::MoverControl:
      control = std::make_unique<MoverWidget>(*this, state, controlMode, key);
      nameLabel = nullptr;
      break;
  }

  control->SetFadeDownSpeed(MapSliderToSpeed(GetFadeOutValue()));
  control->SetFadeUpSpeed(MapSliderToSpeed(GetFadeInValue()));
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
    case FaderControlType::ComboButton:
    case FaderControlType::MoverControl:
      if (newToggleColumn) {
        Gtk::Box &new_box = column.emplace_back(Gtk::Orientation::VERTICAL);
        _controlGrid.attach(new_box, hpos * 2 + 1, vpos, 2, 1);
        new_box.set_vexpand(false);
        new_box.set_valign(Gtk::Align::START);
        column.back().show();
      }
      column.back().append(*control);
      break;
  }

  control->show();
  controls.emplace_back(std::move(control));
}

void FaderWindow::removeFader() {
  FaderState &state = *_state->faders.back();
  const bool hasLower = GetLayout() == "dual";
  if (state.GetFaderType() != FaderControlType::Fader &&
      state.NewToggleButtonColumn()) {
    _upperColumns.pop_back();
    if (hasLower) _lowerColumns.pop_back();
  }
  if (state.GetFaderType() == FaderControlType::Fader) {
    FaderWidget &fader = static_cast<FaderWidget &>(*_upperControls.back());
    _controlGrid.remove(fader.NameLabel());
  }
  _controlGrid.remove(*_upperControls.back());
  _upperControls.pop_back();
  if (hasLower) {
    _controlGrid.remove(*_lowerControls.back());
    _lowerControls.pop_back();
  }
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
  const bool hasLower = GetLayout() == "dual";
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
  const bool hasLower = GetLayout() == "dual";
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

void FaderWindow::onSoloToggled(bool new_value) {
  if (_recursionLock.IsFirst()) {
    _state->isSolo = new_value;
  }
}

void FaderWindow::onControlValueChanged(ControlWidget *widget) {
  if (GetSolo()) {
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
      const bool isLower =
          widget->GetMode() == ControlMode::Secondary && GetLayout() == "dual";
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
    if (GetLayout() == "dual") {
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
                            Gtk::MessageType::QUESTION,
                            Gtk::ButtonsType::OK_CANCEL);
  std::shared_ptr<Gtk::Entry> entry = std::make_shared<Gtk::Entry>();
  dialog.get_message_area()->append(*entry);
  dialog.set_secondary_text("Please enter a name for this fader setup");
  dialog.signal_response().connect([this, entry](int response) {
    if (response == Gtk::ResponseType::OK) {
      _state->name = entry->get_text();
      set_title(_state->name);
      Instance::State().EmitFaderSetChangeSignal();
    }
  });
  dialog.show();
}

void FaderWindow::onChangeDownSpeed() {
  _state->fadeOutSpeed = GetFadeOutValue();
  const double speed = MapSliderToSpeed(_state->fadeOutSpeed);

  for (std::unique_ptr<ControlWidget> &cw : _upperControls)
    cw->SetFadeDownSpeed(speed);
}

void FaderWindow::onChangeUpSpeed() {
  _state->fadeInSpeed = GetFadeInValue();
  const double speed = MapSliderToSpeed(_state->fadeInSpeed);

  for (std::unique_ptr<ControlWidget> &cw : _upperControls)
    cw->SetFadeUpSpeed(speed);
}

void FaderWindow::UpdateValues() {
  if (_connectedInputUniverse || _connectedMidiManager) {
    const size_t n = _upperControls.size();
    _inputValues.resize(n);
    _previousInputValues.resize(n);
    if (_connectedInputUniverse) {
      Instance::Management().GetUniverses().GetInputValues(
          *_connectedInputUniverse, _inputValues.data(), n);
    } else {
      _connectedMidiManager
          ->Update();  // TODO This should move to a higher level
      for (size_t i = 0; i != std::min(n, _connectedMidiManager->GetNFaders());
           ++i) {
        unsigned char value = _connectedMidiManager->GetFaderValue(i);
        _inputValues[i] = std::min(value * 2, 255);
      }
    }
    // Set the faders according to the input
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
        if (color_button_index < 2 && _connectedMidiManager) {
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
        source->CrossFader().Set(0.0, MapSliderToSpeed(GetFadeInValue()));
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

void FaderWindow::onLayoutChanged(const std::string &new_value) {
  if (_recursionLock.IsFirst()) {
    RecursionLock::Token token(_recursionLock);
    layout_action_->set_state(Glib::Variant<Glib::ustring>::create(new_value));
    if (new_value == "primary") {
      _state->mode = FaderSetMode::Primary;
    } else if (new_value == "secondary")
      _state->mode = FaderSetMode::Secondary;
    else  // new_value == "dual"
      _state->mode = FaderSetMode::Dual;
    loadState();
  }
}

void FaderWindow::onInputDeviceClicked() {
  _connectedInputUniverse.reset();
  dialog_ = std::make_unique<StringInputDialog>(
      "Connect input universe to faders",
      "Enter universe number:\n(1 = first universe, leave empty to disconnect)",
      "1");
  dialog_->signal_response().connect([this](int response) {
    if (response == Gtk::ResponseType::OK) {
      StringInputDialog &string_dialog(
          static_cast<StringInputDialog &>(*dialog_));
      const size_t value = std::atoi(string_dialog.Value().c_str());
      if (value >= 1 &&
          value <= Instance::Management().GetUniverses().NUniverses())
        _connectedInputUniverse = value - 1;
    }
    dialog_.reset();
  });
  dialog_->show();
}

std::unique_ptr<ControlMenu> &FaderWindow::GetControlMenu() {
  return control_menu_;
}

}  // namespace glight::gui
