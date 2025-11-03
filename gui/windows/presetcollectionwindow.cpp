#include "presetcollectionwindow.h"

#include "gui/eventtransmitter.h"
#include "gui/instance.h"

#include "theatre/management.h"
#include "theatre/presetcollection.h"

#include <gtkmm/messagedialog.h>

namespace glight::gui {

PresetCollectionWindow::PresetCollectionWindow(
    theatre::PresetCollection &presetCollection)
    : PropertiesWindow(),
      _inputSelector(),
      _controlValueLabel("Value:"),
      _presetCollection(&presetCollection) {
  update_connection_ = Instance::Events().SignalUpdateControllables().connect(
      sigc::mem_fun(*this, &PresetCollectionWindow::onUpdateControllables));

  set_title("glight - " + presetCollection.Name());

  _inputSelector.SignalSelectionChange().connect(
      sigc::mem_fun(*this, &PresetCollectionWindow::onInputSelectionChanged));
  _topBox.append(_inputSelector);
  _inputSelector.set_size_request(200, 200);

  _addPresetButton.set_image_from_icon_name("go-next");
  _addPresetButton.set_sensitive(false);
  _addPresetButton.signal_clicked().connect(
      sigc::mem_fun(*this, &PresetCollectionWindow::onAddPreset));
  _buttonBox.append(_addPresetButton);

  _removePresetButton.set_image_from_icon_name("go-previous");
  _removePresetButton.signal_clicked().connect(
      sigc::mem_fun(*this, &PresetCollectionWindow::onRemovePreset));
  _buttonBox.append(_removePresetButton);

  _buttonBox.set_valign(Gtk::Align::CENTER);
  _topBox.append(_buttonBox);

  _presetsStore = Gtk::ListStore::create(_presetListColumns);

  _presetsView.set_model(_presetsStore);
  _presetsView.append_column("Controllable", _presetListColumns._control);
  _presetsView.append_column("Value", _presetListColumns._value);
  fillPresetsList();
  _presetsView.get_selection()->signal_changed().connect(
      sigc::mem_fun(*this, &PresetCollectionWindow::onSelectedPresetChanged));
  _presetsScrolledWindow.set_child(_presetsView);

  _presetsScrolledWindow.set_size_request(200, 200);
  _presetsScrolledWindow.set_policy(Gtk::PolicyType::NEVER,
                                    Gtk::PolicyType::AUTOMATIC);
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

  _topBox.append(_grid);

  set_child(_topBox);

  load();
  onSelectedPresetChanged();
}

void PresetCollectionWindow::load() { fillPresetsList(); }

theatre::FolderObject &PresetCollectionWindow::GetObject() {
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
  _presetsView.get_selection()->select(
      _presetsStore->children()[index].get_iter());
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
    const std::unique_ptr<theatre::PresetValue> &pValue =
        _presetCollection->PresetValues()[i];
    Gtk::TreeModel::iterator iter = _presetsStore->append();
    Gtk::TreeModel::Row &row = *iter;
    row[_presetListColumns._control] =
        pValue->GetControllable().InputName(pValue->InputIndex());
    std::ostringstream str;
    str << pValue->Value().RoundedPercentage();
    row[_presetListColumns._value] = str.str();
    row[_presetListColumns._presetIndex] = i;
    if (hasSelection && i == index) {
      _presetsView.get_selection()->select(row.get_iter());
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
  theatre::Controllable *object = _inputSelector.SelectedObject();
  size_t input = _inputSelector.SelectedInput();
  if (object && input != InputSelectWidget::NO_INPUT_SELECTED) {
    theatre::Management &management = Instance::Management();
    std::unique_lock<std::mutex> lock(management.Mutex());
    theatre::PresetValue &preset =
        _presetCollection->AddPresetValue(*object, input);
    if (management.HasCycle()) {
      _presetCollection->RemovePresetValue(_presetCollection->Size() - 1);
      lock.unlock();
      Gtk::MessageDialog dialog(
          "Can not add this object to the time sequence: "
          "this would create a cycle in the connections.",
          false, Gtk::MessageType::ERROR);
      dialog.show();
    } else {
      preset.SetValue(theatre::ControlValue::Max());
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
    std::unique_lock<std::mutex> lock(Instance::Management().Mutex());
    _presetCollection->RemovePresetValue(index);
    lock.unlock();

    fillPresetsList();
  }
}

void PresetCollectionWindow::onSelectedPresetChanged() {
  if (_recursionLock.IsFirst()) {
    size_t index = 0;
    if (selectedPresetIndex(index)) {
      loadPreset(index);
      setPresetSensitive(true);
    } else {
      setPresetSensitive(false);
    }
  }
}

void PresetCollectionWindow::loadPreset(size_t index) {
  std::unique_lock<std::mutex> lock(Instance::Management().Mutex());
  theatre::ControlValue controlValue =
      _presetCollection->PresetValues()[index]->Value();
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
  if (Instance::Management().Contains(*_presetCollection))
    load();
  else
    hide();
}

void PresetCollectionWindow::onControlValueChanged() {
  std::unique_lock<std::mutex> lock(Instance::Management().Mutex());
  size_t index = 0;
  if (selectedPresetIndex(index)) {
    double percentage = std::atof(_controlValueEntry.get_text().c_str());
    if (percentage >= 0.0 && percentage <= 100.0) {
      theatre::ControlValue controlValue(
          percentage * theatre::ControlValue::MaxUInt() / 100.0);
      _presetCollection->PresetValues()[index]->SetValue(controlValue);
    }
  }
  lock.unlock();
  fillPresetsList();
}

}  // namespace glight::gui
