#include "presetcollectionwindow.h"

#include "../eventtransmitter.h"

#include "../../theatre/management.h"

#include <gtkmm/messagedialog.h>

PresetCollectionWindow::PresetCollectionWindow(
    PresetCollection &presetCollection, Management &management,
    EventTransmitter &eventHub)
    : PropertiesWindow(),
      _inputSelector(management, eventHub),

      _controlValueLabel("Value:"),
      _controlValueEntry(),

      _presetCollection(&presetCollection),
      _management(&management),
      _eventHub(eventHub) {
  _changeManagementConnection = eventHub.SignalChangeManagement().connect(
      sigc::mem_fun(*this, &PresetCollectionWindow::onChangeManagement));
  _updateControllablesConnection = eventHub.SignalUpdateControllables().connect(
      sigc::mem_fun(*this, &PresetCollectionWindow::onUpdateControllables));

  set_title("glight - " + presetCollection.Name());

  _inputSelector.SignalSelectionChange().connect(
      sigc::mem_fun(*this, &PresetCollectionWindow::onInputSelectionChanged));
  _topBox.pack_start(_inputSelector);
  _inputSelector.set_size_request(200, 200);

  _addPresetButton.set_image_from_icon_name("go-next");
  _addPresetButton.set_sensitive(false);
  _addPresetButton.signal_clicked().connect(
      sigc::mem_fun(*this, &PresetCollectionWindow::onAddPreset));
  _buttonBox.pack_start(_addPresetButton, false, false, 4);

  _removePresetButton.set_image_from_icon_name("go-previous");
  _removePresetButton.signal_clicked().connect(
      sigc::mem_fun(*this, &PresetCollectionWindow::onRemovePreset));
  _buttonBox.pack_start(_removePresetButton, false, false, 4);

  _buttonBox.set_valign(Gtk::ALIGN_CENTER);
  _topBox.pack_start(_buttonBox, false, false, 4);

  _presetsStore = Gtk::ListStore::create(_presetListColumns);

  _presetsView.set_model(_presetsStore);
  _presetsView.append_column("Controllable", _presetListColumns._control);
  _presetsView.append_column("Value", _presetListColumns._value);
  fillPresetsList();
  _presetsView.get_selection()->signal_changed().connect(
      sigc::mem_fun(*this, &PresetCollectionWindow::onSelectedPresetChanged));
  _presetsScrolledWindow.add(_presetsView);

  _presetsScrolledWindow.set_size_request(200, 200);
  _presetsScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  _grid.attach(_presetsScrolledWindow, 0, 0, 2, 1);
  _presetsScrolledWindow.set_hexpand(true);
  _presetsScrolledWindow.set_vexpand(true);

  _grid.attach(_controlValueLabel, 0, 1, 1, 1);
  _controlValueLabel.set_hexpand(false);
  _controlValueLabel.set_vexpand(false);

  _controlValueEntry.signal_changed().connect(
      [&]() { onControlValueChanged(); });
  _grid.attach(_controlValueEntry, 1, 1, 1, 1);
  _controlValueEntry.set_hexpand(false);
  _controlValueEntry.set_vexpand(false);

  _topBox.pack_end(_grid, true, true, 4);

  add(_topBox);
  show_all_children();

  load();
  onSelectedPresetChanged();
}

PresetCollectionWindow::~PresetCollectionWindow() {
  _changeManagementConnection.disconnect();
  _updateControllablesConnection.disconnect();
}

void PresetCollectionWindow::load() { fillPresetsList(); }

FolderObject &PresetCollectionWindow::GetObject() {
  return GetPresetCollection();
}

bool PresetCollectionWindow::selectedPresetIndex(size_t &index) {
  Gtk::TreeModel::iterator selIter =
      _presetsView.get_selection()->get_selected();
  if (selIter) {
    index = (*selIter)[_presetListColumns._presetIndex];
    return true;
  } else {
    return false;
  }
}

void PresetCollectionWindow::selectPreset(size_t index) {
  _presetsView.get_selection()->select(_presetsStore->children()[index]);
}

void PresetCollectionWindow::fillPresetsList() {
  RecursionLock::Token token(_recursionLock);
  Gtk::TreeModel::iterator iter = _presetsView.get_selection()->get_selected();
  bool hasSelection = false;
  size_t index = 0;
  if (iter) {
    hasSelection = true;
    index = (*iter)[_presetListColumns._presetIndex];
  }
  _presetsStore->clear();
  for (size_t i = 0; i != _presetCollection->PresetValues().size(); ++i) {
    const std::unique_ptr<PresetValue> &pValue =
        _presetCollection->PresetValues()[i];
    Gtk::TreeModel::iterator iter = _presetsStore->append();
    Gtk::TreeModel::Row row = *iter;
    row[_presetListColumns._control] =
        pValue->Controllable().InputName(pValue->InputIndex());
    std::ostringstream str;
    str << pValue->Value().RoundedPercentage();
    row[_presetListColumns._value] = str.str();
    row[_presetListColumns._presetIndex] = i;
    if (hasSelection && i == index) {
      _presetsView.get_selection()->select(row);
    }
  }
  token.Release();
  if (hasSelection && !_presetsView.get_selection()->get_selected())
    onSelectedPresetChanged();
}

void PresetCollectionWindow::onInputSelectionChanged() {
  _addPresetButton.set_sensitive(_inputSelector.HasInputSelected());
}

void PresetCollectionWindow::onAddPreset() {
  Controllable *object = _inputSelector.SelectedObject();
  size_t input = _inputSelector.SelectedInput();
  if (object && input != InputSelectWidget::NO_INPUT_SELECTED) {
    std::unique_lock<std::mutex> lock(_management->Mutex());
    PresetValue &preset = _presetCollection->AddPresetValue(*object, input);
    if (_management->HasCycle()) {
      _presetCollection->RemovePresetValue(_presetCollection->Size() - 1);
      lock.unlock();
      Gtk::MessageDialog dialog(
          "Can not add this object to the time sequence: "
          "this would create a cycle in the connections.",
          false, Gtk::MESSAGE_ERROR);
      dialog.run();
    } else {
      preset.SetValue(ControlValue::MaxUInt());
      lock.unlock();
      fillPresetsList();
      selectPreset(_presetCollection->Size() - 1);
    }
  }
}

void PresetCollectionWindow::onRemovePreset() {
  Gtk::TreeModel::iterator selIter =
      _presetsView.get_selection()->get_selected();
  if (selIter) {
    size_t index = (*selIter)[_presetListColumns._presetIndex];
    std::unique_lock<std::mutex> lock(_management->Mutex());
    _presetCollection->RemovePresetValue(index);
    lock.unlock();

    fillPresetsList();
  }
}

void PresetCollectionWindow::onSelectedPresetChanged() {
  if (_recursionLock.IsFirst()) {
    size_t index;
    if (selectedPresetIndex(index)) {
      loadPreset(index);
      setPresetSensitive(true);
    } else {
      setPresetSensitive(false);
    }
  }
}

void PresetCollectionWindow::loadPreset(size_t index) {
  std::unique_lock<std::mutex> lock(_management->Mutex());
  ControlValue controlValue = _presetCollection->PresetValues()[index]->Value();
  lock.unlock();
  std::ostringstream str;
  str << controlValue.RoundedPercentage();
  _controlValueEntry.set_text(str.str());
}

void PresetCollectionWindow::setPresetSensitive(bool sensitive) {
  _removePresetButton.set_sensitive(sensitive);
  _controlValueEntry.set_sensitive(sensitive);
}

void PresetCollectionWindow::onUpdateControllables() {
  if (_management->Contains(*_presetCollection))
    load();
  else
    hide();
}

void PresetCollectionWindow::onControlValueChanged() {
  std::unique_lock<std::mutex> lock(_management->Mutex());
  size_t index;
  if (selectedPresetIndex(index)) {
    double percentage = std::atof(_controlValueEntry.get_text().c_str());
    if (percentage >= 0.0 && percentage <= 100.0) {
      ControlValue controlValue(percentage * ControlValue::MaxUInt() / 100.0);
      _presetCollection->PresetValues()[index]->SetValue(controlValue);
    }
  }
  lock.unlock();
  fillPresetsList();
}
