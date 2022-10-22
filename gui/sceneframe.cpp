#include <cmath>
#include <sstream>

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/stock.h>

#include <glibmm/main.h>

#include "../theatre/management.h"
#include "../theatre/scene.h"
#include "../theatre/show.h"

#include "sceneframe.h"

namespace glight::gui {

SceneFrame::SceneFrame(theatre::Management &management,
                       ShowWindow &parentWindow)
    : Gtk::Frame("Scene"),
      _management(&management),
      _show(&_management->GetShow()),
      _audioWidget(),
      _clickIsLabel("Click is: "),
      _clickIsSelectButton("select"),
      _clickIsSetStartButton("set start"),
      _clickIsSetEndButton("set end"),
      _clickIsAddKeyButton("add key"),
      _clickIsAddItemButton("add item"),
      _audioLabel("Audio file: -"),
      _changeAudioButton("Change"),
      _startButton(Gtk::Stock::MEDIA_PLAY),
      _startSelectionButton("Play selection"),
      _stopButton(Gtk::Stock::MEDIA_STOP),
      _key1Button("Key-1"),
      _createControlItemButton(Gtk::Stock::ADD),
      _setEndTimeButton("Set end time"),
      _removeButton(Gtk::Stock::REMOVE),
      _createTransitionItemButton("Add transition"),
      _startScale(0, theatre::ControlValue::MaxUInt() + 1,
                  theatre::ControlValue::MaxUInt() / 100.0),
      _endScale(0, theatre::ControlValue::MaxUInt() + 1,
                theatre::ControlValue::MaxUInt() / 100.0),
      _nameFrame(management, parentWindow),
      _selectedScene(nullptr),
      _isUpdating(false) {
  _audioWidget.SignalClicked().connect(
      sigc::mem_fun(*this, &SceneFrame::onAudioWidgetClicked));
  _vBox.pack_start(_audioWidget);
  _audioWidget.show();

  _audioBox.pack_start(_clickIsLabel, false, false);
  _clickIsLabel.show();

  Gtk::RadioButtonGroup clickIsGroup;
  _clickIsSelectButton.set_group(clickIsGroup);
  _audioBox.pack_start(_clickIsSelectButton, false, false);
  _clickIsSelectButton.show();

  _clickIsSetStartButton.set_group(clickIsGroup);
  _audioBox.pack_start(_clickIsSetStartButton, false, false);
  _clickIsSetStartButton.show();

  _clickIsSetEndButton.set_group(clickIsGroup);
  _audioBox.pack_start(_clickIsSetEndButton, false, false);
  _clickIsSetEndButton.show();

  _clickIsAddKeyButton.set_group(clickIsGroup);
  _audioBox.pack_start(_clickIsAddKeyButton, false, false);
  _clickIsAddKeyButton.show();

  _clickIsAddItemButton.set_group(clickIsGroup);
  _audioBox.pack_start(_clickIsAddItemButton, false, false);
  _clickIsAddItemButton.show();

  _audioBox.pack_start(_audioLabel);
  _audioLabel.show();

  _changeAudioButton.signal_clicked().connect(
      sigc::mem_fun(*this, &SceneFrame::onChangeAudioButtonPressed));
  _audioBox.pack_start(_changeAudioButton, false, false, 5);
  _changeAudioButton.show();

  _vBox.pack_start(_audioBox, false, false);
  _audioBox.show();

  createSceneItemsList();

  createControllablesList1();

  _startButton.signal_clicked().connect(
      sigc::mem_fun(*this, &SceneFrame::onStartButtonPressed));
  _sceneItemUButtonBox.pack_start(_startButton);
  _startButton.show();

  _startSelectionButton.signal_clicked().connect(
      sigc::mem_fun(*this, &SceneFrame::onStartSelectionButtonPressed));
  _sceneItemUButtonBox.pack_start(_startSelectionButton);
  _startSelectionButton.show();

  _stopButton.signal_clicked().connect(
      sigc::mem_fun(*this, &SceneFrame::onStopButtonPressed));
  _sceneItemUButtonBox.pack_start(_stopButton);
  _stopButton.show();

  _key1Button.signal_clicked().connect(
      sigc::mem_fun(*this, &SceneFrame::onKey1ButtonPressed));
  _sceneItemUButtonBox.pack_start(_key1Button);
  _key1Button.show();

  _createControlItemButton.signal_clicked().connect(
      sigc::mem_fun(*this, &SceneFrame::onCreateControlItemButtonPressed));
  _createControlItemButton.set_sensitive(false);
  _sceneItemUButtonBox.pack_start(_createControlItemButton);
  _createControlItemButton.show();

  _setEndTimeButton.signal_clicked().connect(
      sigc::mem_fun(*this, &SceneFrame::onSetEndTimeButtonPressed));
  _setEndTimeButton.set_sensitive(false);
  _sceneItemUButtonBox.pack_start(_setEndTimeButton);
  _setEndTimeButton.show();

  _removeButton.signal_clicked().connect(
      sigc::mem_fun(*this, &SceneFrame::onRemoveButtonPressed));
  _removeButton.set_sensitive(false);
  _sceneItemUButtonBox.pack_start(_removeButton);
  _removeButton.show();

  _sceneItemBox.pack_start(_sceneItemUButtonBox, false, false, 2);
  _sceneItemUButtonBox.show();

  _startScale.set_inverted(true);
  _startScale.set_draw_value(false);
  _startScale.set_sensitive(false);
  _startScale.signal_value_changed().connect(
      sigc::mem_fun(*this, &SceneFrame::onScalesChanged));
  _scalesBox.pack_start(_startScale);
  _startScale.show();

  _endScale.set_inverted(true);
  _endScale.set_draw_value(false);
  _endScale.set_sensitive(false);
  _endScale.signal_value_changed().connect(
      sigc::mem_fun(*this, &SceneFrame::onScalesChanged));
  _scalesBox.pack_start(_endScale);
  _endScale.show();

  _sceneItemBox.pack_start(_scalesBox);
  _scalesBox.show();

  createControllablesList2();

  _hBox.pack_start(_sceneItemBox, false, false, 2);
  _sceneItemBox.show();

  _vBox.pack_start(_hBox);
  _hBox.show();

  add(_vBox);
  _vBox.show();

  _timeoutConnection = Glib::signal_timeout().connect(
      sigc::mem_fun(*this, &SceneFrame::onTimeout), 20);
}

SceneFrame::~SceneFrame() { _timeoutConnection.disconnect(); }

void SceneFrame::Update() {
  if (&_management->GetShow() != _show) {
    _show = &_management->GetShow();
    _selectedScene = nullptr;
  }
  // For testing
  if (_show->Scenes().empty())
    _selectedScene = _show->AddScene(true);
  else
    _selectedScene = _show->Scenes()[0].get();
  _audioWidget.SetScene(*_selectedScene);

  // fillControllablesList();
  fillSceneItemList();
  updateAudio();
  updateAudioWidgetKeys();
}

void SceneFrame::createSceneItemsList() {
  _sceneItemsListModel = Gtk::ListStore::create(_sceneItemsListColumns);

  _sceneItemsListView.set_model(_sceneItemsListModel);
  _sceneItemsListView.append_column("Start (s)",
                                    _sceneItemsListColumns._startTime);
  _sceneItemsListView.append_column("Dur (s)", _sceneItemsListColumns._endTime);
  _sceneItemsListView.append_column("S-Value",
                                    _sceneItemsListColumns._startValue);
  _sceneItemsListView.append_column("E-Value",
                                    _sceneItemsListColumns._endValue);
  _sceneItemsListView.append_column("Description",
                                    _sceneItemsListColumns._description);
  _sceneItemsListView.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
  _sceneItemsListView.get_selection()->signal_changed().connect(
      sigc::mem_fun(*this, &SceneFrame::onSelectedSceneItemChanged));
  _sceneItemsListView.set_rubber_banding(true);
  _listScrolledWindow.add(_sceneItemsListView);
  _sceneItemsListView.show();

  _listScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  _hBox.pack_start(_listScrolledWindow, true, true);
  _listScrolledWindow.show();
}

void SceneFrame::createControllablesList1() {
  _controllablesListModel = Gtk::ListStore::create(_controllablesListColumns);

  _controllables1ComboBox.set_model(_controllablesListModel);
  _controllables1ComboBox.pack_start(_controllablesListColumns._text);

  _sceneItemBox.pack_start(_controllables1ComboBox, false, false);
  _controllables1ComboBox.show();
}

void SceneFrame::createControllablesList2() {
  _controllables2ComboBox.set_model(_controllablesListModel);
  _controllables2ComboBox.pack_start(_controllablesListColumns._text);

  _sceneItemBox.pack_start(_controllables2ComboBox, false, false);
  _controllables2ComboBox.show();
}

void SceneFrame::fillSceneItemList() {
  _sceneItemsListModel->clear();

  if (_selectedScene != nullptr) {
    std::unique_lock<std::mutex> lock(_management->Mutex());
    const std::multimap<double, std::unique_ptr<theatre::SceneItem>> &items =
        _selectedScene->SceneItems();
    for (const std::pair<const double, std::unique_ptr<theatre::SceneItem>>
             &item : items) {
      Gtk::TreeModel::iterator iter = _sceneItemsListModel->append();
      Gtk::TreeRow row = *iter;
      setSceneItemListRow(item.second.get(), row);
    }
    lock.unlock();
    _sceneItemsListView.set_sensitive(true);
  } else {
    _sceneItemsListView.set_sensitive(false);
  }
}

void SceneFrame::setSceneItemListRow(theatre::SceneItem *sceneItem,
                                     Gtk::TreeModel::Row row) {
  std::stringstream startTimeStr;
  startTimeStr << (round(sceneItem->OffsetInMS() / 10.0) / 100.0);
  row[_sceneItemsListColumns._startTime] = startTimeStr.str();
  if (dynamic_cast<const theatre::KeySceneItem *>(sceneItem) == nullptr) {
    std::stringstream endTimeStr;
    endTimeStr << (round(sceneItem->DurationInMS() / 10.0) / 100.0);
    row[_sceneItemsListColumns._endTime] = endTimeStr.str();

    const theatre::ControlSceneItem *csi =
        dynamic_cast<const theatre::ControlSceneItem *>(sceneItem);
    if (csi != nullptr) {
      std::stringstream sValStr;
      sValStr << (round(csi->StartValue().Ratio() * 1000.0) / 10.0) << "%";
      row[_sceneItemsListColumns._startValue] = sValStr.str();

      std::stringstream eValStr;
      eValStr << (round(csi->EndValue().Ratio() * 1000.0) / 10.0) << "%";
      row[_sceneItemsListColumns._endValue] = eValStr.str();
    }
  }
  row[_sceneItemsListColumns._description] = sceneItem->Description();
  row[_sceneItemsListColumns._item] = sceneItem;
}

void SceneFrame::updateSelectedSceneItems() {
  Glib::RefPtr<Gtk::TreeSelection> selection =
      _sceneItemsListView.get_selection();
  std::vector<Gtk::TreeModel::Path> pathHandle = selection->get_selected_rows();
  std::lock_guard<std::mutex> lock(_management->Mutex());
  for (std::vector<Gtk::TreeModel::Path>::iterator pathPtr = pathHandle.begin();
       pathPtr != pathHandle.end(); ++pathPtr) {
    Gtk::TreeModel::iterator iter = _sceneItemsListModel->get_iter(*pathPtr);
    Gtk::TreeModel::Row row = *iter;
    theatre::SceneItem *item = row[_sceneItemsListColumns._item];
    setSceneItemListRow(item, row);
  }
}

void SceneFrame::fillControllablesList() {
  _controllablesListModel->clear();

  std::lock_guard<std::mutex> lock(_management->Mutex());
  const std::vector<std::unique_ptr<theatre::Controllable>> &controllable =
      _management->Controllables();
  for (const std::unique_ptr<theatre::Controllable> &contr : controllable) {
    Gtk::TreeModel::iterator iter = _controllablesListModel->append();
    Gtk::TreeModel::Row row = *iter;
    row[_controllablesListColumns._text] = contr->Name();
    row[_controllablesListColumns._controllable] = contr.get();
  }
}

void SceneFrame::onStartButtonPressed() {
  if (_selectedScene != nullptr) {
    std::lock_guard<std::mutex> lock(_management->Mutex());
    _selectedScene->SetStartOffset(0.0);
    _show->StartScene(_management->GetOffsetTimeInMS(), _selectedScene);
  }
}

void SceneFrame::onStartSelectionButtonPressed() {
  if (_selectedScene != nullptr) {
    std::lock_guard<std::mutex> lock(_management->Mutex());
    _selectedScene->SetStartOffset(_audioWidget.Position());
    _show->StartScene(_management->GetOffsetTimeInMS(), _selectedScene);
  }
}

void SceneFrame::onStopButtonPressed() {
  if (_selectedScene != nullptr) {
    std::lock_guard<std::mutex> lock(_management->Mutex());
    _show->StopScene(_selectedScene);
  }
}

void SceneFrame::addKey(theatre::KeySceneLevel level) {
  std::unique_lock<std::mutex> lock(_management->Mutex());
  theatre::KeySceneItem *key = _selectedScene->AddKeySceneItem(
      _management->GetOffsetTimeInMS() - _selectedScene->StartTimeInMS());
  key->SetLevel(level);
  lock.unlock();

  fillSceneItemList();
  updateAudioWidgetKeys();
}

void SceneFrame::onKey1ButtonPressed() { addKey(theatre::KeySceneLevel::Key); }

void SceneFrame::onCreateControlItemButtonPressed() {
  Gtk::TreeModel::iterator activeControllable =
      _controllables1ComboBox.get_active();
  if (activeControllable) {
    _isUpdating = true;

    Glib::RefPtr<Gtk::TreeSelection> selection =
        _sceneItemsListView.get_selection();
    std::vector<Gtk::TreeModel::Path> pathHandle =
        selection->get_selected_rows();
    std::unique_lock<std::mutex> lock(_management->Mutex());
    for (std::vector<Gtk::TreeModel::Path>::const_iterator pathPtr =
             pathHandle.begin();
         pathPtr != pathHandle.end(); ++pathPtr) {
      theatre::SceneItem *selItem = (*_sceneItemsListModel->get_iter(
          *pathPtr))[_sceneItemsListColumns._item];
      theatre::SceneItem *nextItem = nullptr;
      std::vector<Gtk::TreeModel::Path>::const_iterator nextPtr = pathPtr;
      ++nextPtr;
      if (nextPtr != pathHandle.end())
        nextItem = (*_sceneItemsListModel->get_iter(
            *nextPtr))[_sceneItemsListColumns._item];

      theatre::ControlSceneItem *item = _selectedScene->AddControlSceneItem(
          selItem->OffsetInMS(),
          *(*activeControllable)[_controllablesListColumns._controllable], 0);
      if (nextItem != nullptr)
        item->SetDurationInMS(nextItem->OffsetInMS() - item->OffsetInMS());
      else
        item->SetDurationInMS(1000);
    }
    lock.unlock();

    fillSceneItemList();
    updateAudioWidgetKeys();
    _isUpdating = false;
    onSelectedSceneItemChanged();
  }
}

void SceneFrame::onSelectedSceneItemChanged() {
  if (!_isUpdating) {
    switch (selectedSceneItemCount()) {
      case 0:
        _createControlItemButton.set_sensitive(false);
        _setEndTimeButton.set_sensitive(false);
        _removeButton.set_sensitive(false);
        _startScale.set_sensitive(false);
        _endScale.set_sensitive(false);
        break;
      case 1: {
        _createControlItemButton.set_sensitive(true);
        _setEndTimeButton.set_sensitive(true);
        _removeButton.set_sensitive(true);
        _startScale.set_sensitive(true);
        _endScale.set_sensitive(true);
        std::unique_lock<std::mutex> lock(_management->Mutex());
        theatre::SceneItem *item = selectedItem();
        theatre::ControlSceneItem *csi =
            dynamic_cast<theatre::ControlSceneItem *>(item);
        if (csi != nullptr) {
          unsigned s = csi->StartValue().UInt(), e = csi->EndValue().UInt();
          lock.unlock();
          _startScale.set_value(s);
          _endScale.set_value(e);
        }
        _audioWidget.SetPosition(item->OffsetInMS());
      } break;
      default:
        _createControlItemButton.set_sensitive(true);
        _setEndTimeButton.set_sensitive(true);
        _removeButton.set_sensitive(true);
        _startScale.set_sensitive(true);
        _endScale.set_sensitive(true);
        break;
    }
  }
}

void SceneFrame::onSetEndTimeButtonPressed() {
  Glib::RefPtr<Gtk::TreeSelection> selection =
      _sceneItemsListView.get_selection();
  std::vector<Gtk::TreeModel::Path> pathHandle = selection->get_selected_rows();
  std::unique_lock<std::mutex> lock(_management->Mutex());
  for (std::vector<Gtk::TreeModel::Path>::const_iterator pathPtr =
           pathHandle.begin();
       pathPtr != pathHandle.end(); ++pathPtr) {
    theatre::SceneItem *selItem = (*_sceneItemsListModel->get_iter(
                           *pathPtr))[_sceneItemsListColumns._item],
                       *nextItem = nullptr;
    std::vector<Gtk::TreeModel::Path>::const_iterator nextPtr = pathPtr;
    ++nextPtr;
    if (nextPtr != pathHandle.end()) {
      nextItem = (*_sceneItemsListModel->get_iter(
          *nextPtr))[_sceneItemsListColumns._item];
      selItem->SetDurationInMS(nextItem->OffsetInMS() - selItem->OffsetInMS());
    }
  }
  lock.unlock();

  updateAudioWidgetKeys();
  updateSelectedSceneItems();
}

void SceneFrame::onRemoveButtonPressed() {
  _isUpdating = true;
  Glib::RefPtr<Gtk::TreeSelection> selection =
      _sceneItemsListView.get_selection();
  std::vector<Gtk::TreeModel::Path> pathHandle = selection->get_selected_rows();
  std::unique_lock<std::mutex> lock(_management->Mutex());
  for (std::vector<Gtk::TreeModel::Path>::const_iterator pathPtr =
           pathHandle.begin();
       pathPtr != pathHandle.end(); ++pathPtr) {
    theatre::SceneItem *item = (*_sceneItemsListModel->get_iter(
        *pathPtr))[_sceneItemsListColumns._item];
    _selectedScene->Remove(item);
  }
  lock.unlock();
  fillSceneItemList();
  updateAudioWidgetKeys();
  _isUpdating = false;
  onSelectedSceneItemChanged();
}

bool SceneFrame::HandleKeyDown(char key) {
  using theatre::KeySceneLevel;
  switch (key) {
    case '=':
      onStartButtonPressed();
      return true;
    case '1':
      addKey(KeySceneLevel::Section);
      return true;
    case '2':
      addKey(KeySceneLevel::Measure);
      return true;
    case '3':
      addKey(KeySceneLevel::Highlight);
      return true;
    case '4':
      addKey(KeySceneLevel::Beat);
      return true;
    case '5':
      addKey(KeySceneLevel::Key);
      return true;
  }
  return false;
}

void SceneFrame::onScalesChanged() {
  if (!_isUpdating) {
    _isUpdating = true;
    Glib::RefPtr<Gtk::TreeSelection> selection =
        _sceneItemsListView.get_selection();
    std::vector<Gtk::TreeModel::Path> pathHandle =
        selection->get_selected_rows();
    std::unique_lock<std::mutex> lock(_management->Mutex());
    for (std::vector<Gtk::TreeModel::Path>::const_iterator pathPtr =
             pathHandle.begin();
         pathPtr != pathHandle.end(); ++pathPtr) {
      theatre::SceneItem *item = (*_sceneItemsListModel->get_iter(
          *pathPtr))[_sceneItemsListColumns._item];
      theatre::ControlSceneItem *csi =
          dynamic_cast<theatre::ControlSceneItem *>(item);
      if (csi != nullptr) {
        csi->StartValue().Set((unsigned)_startScale.get_value());
        csi->EndValue().Set((unsigned)_endScale.get_value());
      }
    }
    lock.unlock();
    updateSelectedSceneItems();
    _isUpdating = false;
  }
}

void SceneFrame::onChangeAudioButtonPressed() {
  Gtk::FileChooserDialog dialog("Open audio file",
                                Gtk::FILE_CHOOSER_ACTION_OPEN);

  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button("Open", Gtk::RESPONSE_OK);

  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name("Flac audio file");
  filter->add_pattern("*.flac");
  filter->add_mime_type("audio/flac");
  dialog.add_filter(filter);

  int result = dialog.run();
  if (result == Gtk::RESPONSE_OK) {
    _selectedScene->SetAudioFile(dialog.get_filename());
    updateAudio();
  }
}

bool SceneFrame::onTimeout() {
  std::unique_lock<std::mutex> lock(_management->Mutex());
  if (_selectedScene != nullptr && _selectedScene->IsPlaying()) {
    double pos =
        _management->GetOffsetTimeInMS() - _selectedScene->StartTimeInMS();
    lock.unlock();
    _audioWidget.SetPosition(pos);
  }
  return true;
}

void SceneFrame::onAudioWidgetClicked(double timeInMS) {
  if (_clickIsSetStartButton.get_active()) {
    _isUpdating = true;
    Glib::RefPtr<Gtk::TreeSelection> selection =
        _sceneItemsListView.get_selection();
    std::vector<Gtk::TreeModel::Path> pathHandle =
        selection->get_selected_rows();
    std::unique_lock<std::mutex> lock(_management->Mutex());
    for (std::vector<Gtk::TreeModel::Path>::const_iterator pathPtr =
             pathHandle.begin();
         pathPtr != pathHandle.end(); ++pathPtr) {
      theatre::SceneItem *item = (*_sceneItemsListModel->get_iter(
          *pathPtr))[_sceneItemsListColumns._item];
      _selectedScene->ChangeSceneItemStartTime(item, timeInMS);
    }
    lock.unlock();
    _isUpdating = false;
    updateSelectedSceneItems();
    updateAudioWidgetKeys();
  }

  else if (_clickIsSetEndButton.get_active()) {
    _isUpdating = true;
    Glib::RefPtr<Gtk::TreeSelection> selection =
        _sceneItemsListView.get_selection();
    std::vector<Gtk::TreeModel::Path> pathHandle =
        selection->get_selected_rows();
    std::unique_lock<std::mutex> lock(_management->Mutex());
    for (std::vector<Gtk::TreeModel::Path>::const_iterator pathPtr =
             pathHandle.begin();
         pathPtr != pathHandle.end(); ++pathPtr) {
      theatre::SceneItem *item = (*_sceneItemsListModel->get_iter(
          *pathPtr))[_sceneItemsListColumns._item];
      item->SetDurationInMS(timeInMS - item->OffsetInMS());
    }
    lock.unlock();
    _isUpdating = false;
    updateSelectedSceneItems();
    updateAudioWidgetKeys();
  }

  else if (_clickIsAddKeyButton.get_active()) {
    _isUpdating = true;

    std::unique_lock<std::mutex> lock(_management->Mutex());
    theatre::KeySceneItem *item = _selectedScene->AddKeySceneItem(timeInMS);
    item->SetLevel(theatre::KeySceneLevel::Key);
    lock.unlock();

    fillSceneItemList();
    updateAudioWidgetKeys();
    _isUpdating = false;
    onSelectedSceneItemChanged();
  } else if (_clickIsAddItemButton.get_active()) {
    Gtk::TreeModel::iterator activeControllable =
        _controllables1ComboBox.get_active();
    if (activeControllable) {
      _isUpdating = true;

      std::unique_lock<std::mutex> lock(_management->Mutex());
      theatre::ControlSceneItem *item = _selectedScene->AddControlSceneItem(
          timeInMS,
          *(*activeControllable)[_controllablesListColumns._controllable], 0);
      item->SetDurationInMS(1000);
      lock.unlock();

      fillSceneItemList();
      updateAudioWidgetKeys();
      _isUpdating = false;
      onSelectedSceneItemChanged();
    }
  } else
    _audioWidget.SetPosition(timeInMS);
}

void SceneFrame::updateAudio() {
  std::unique_lock<std::mutex> lock(_management->Mutex());
  if (_selectedScene != nullptr && _selectedScene->AudioFile() != _audioFile) {
    _audioFile = _selectedScene->AudioFile();
    lock.unlock();
    _audioLabel.set_text(std::string("Audio file: ") + _audioFile);
    try {
      system::FlacDecoder decoder(_audioFile);
      std::cout << "Starting decoder" << std::endl;
      decoder.Start();
      _audioWidget.SetAudioData(decoder);
    } catch (std::exception &e) {
      std::cout << e.what() << '\n';
    }
  }
}

void SceneFrame::updateAudioWidgetKeys() {
  std::lock_guard<std::mutex> lock(_management->Mutex());
  _audioWidget.UpdateKeys();
}

}  // namespace glight::gui
