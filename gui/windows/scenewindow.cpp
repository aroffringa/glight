#include <cmath>
#include <iostream>
#include <sstream>

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include <glibmm/main.h>

#include "../../theatre/folder.h"
#include "../../theatre/management.h"
#include "../../theatre/sourcevalue.h"

#include "../../theatre/scenes/scene.h"

#include "../eventtransmitter.h"

#include "../dialogs/inputselectdialog.h"
#include "../dialogs/stringinputdialog.h"
#include "../dialogs/sceneselect.h"

#include "scenewindow.h"

using glight::theatre::Controllable;
using glight::theatre::Scene;

namespace {
Scene *FirstScene(glight::theatre::Management &management) {
  for (const std::unique_ptr<Controllable> &controllable :
       management.Controllables()) {
    if (Scene *scene = dynamic_cast<Scene *>(controllable.get()); scene) {
      return scene;
    }
  }
  return nullptr;
}

}  // namespace

namespace glight::gui {

SceneWindow::SceneWindow(theatre::Management &management,
                         ShowWindow &parentWindow, EventTransmitter &eventHub)
    : _management(management),
      _eventHub(eventHub),
      _audioLabel("Audio file: -"),
      _selectControllableButton("Select..."),
      _createControlItemButton(Gtk::Stock::ADD),
      _setEndTimeButton("Set end time"),
      _removeButton(Gtk::Stock::REMOVE),
      _blackoutButton("Black-out"),
      _restoreButton("Restore"),
      _setFadeSpeedButton("Fade speed"),
      _startScale(0, theatre::ControlValue::MaxUInt() + 1,
                  theatre::ControlValue::MaxUInt() / 100.0),
      _endScale(0, theatre::ControlValue::MaxUInt() + 1,
                theatre::ControlValue::MaxUInt() / 100.0),
      _nameFrame(management, parentWindow),
      _selectedScene(nullptr),
      _sourceValue(nullptr),
      _isUpdating(false) {
  set_default_size(500, 500);

  addTool(new_scene_tb_, "New scene", "Adds a new scene to the current show",
          "document-new", [&]() { NewScene(); });
  addTool(load_scene_tb_, "Load scene", "Load an existing scene",
          "document-open", [&]() { LoadScene(); });

  _toolbar.append(separator1_);
  addTool(rewind_tb_, "Rewind", "Skip to start", "media-skip-backward",
          [&]() { Rewind(); });
  addTool(start_tb_, "Play", "Start from the current position",
          "media-playback-start", [&]() { StartPlayback(); });
  addTool(seek_backward_tb_, "Seek backward", "Jump half a screen backward",
          "media-seek-backward", [&]() { SeekBackward(); });
  addTool(seek_forward_tb_, "Forward", "Jump half a screen forward",
          "media-seek-forward", [&]() { SeekForward(); });
  addTool(change_audio_tb_, "Change audio",
          "Select an audio file for this scene", "media-eject",
          [&]() { ChangeAudio(); });
  _toolbar.append(separator2_);

  Gtk::RadioToolButton::Group group;
  addTool(move_cursor_tb_, "Move cursor",
          "Clicking the audio will move the cursor", "start-here", [&]() {});
  move_cursor_tb_.set_group(group);
  addTool(set_start_tb_, "Set start time",
          "Clicking the audio will change the start time of the selection",
          "go-first", [&]() {});
  set_start_tb_.set_group(group);
  addTool(set_end_tb_, "Set end time",
          "Clicking the audio will change the end time of the selection",
          "go-last", [&]() {});
  set_end_tb_.set_group(group);
  addTool(add_key_tb_, "Add key", "Clicking the audio will add a key",
          "starred", [&]() {});
  add_key_tb_.set_group(group);
  addTool(add_item_tb_, "Add item", "Clicking the audio will add an item",
          "list-add", [&]() {});
  add_item_tb_.set_group(group);

  _vBox.pack_start(_toolbar, false, false);

  _audioWidget.SignalClicked().connect(
      sigc::mem_fun(*this, &SceneWindow::onAudioWidgetClicked));
  _vBox.pack_start(_audioWidget);

  Gtk::RadioButtonGroup clickIsGroup;

  _audioBox.pack_start(_audioLabel);

  _vBox.pack_start(_audioBox, false, false);

  createSceneItemsList();

  createControllablesList();

  _selectControllableButton.signal_clicked().connect(
      sigc::mem_fun(*this, &SceneWindow::onSelectControllable));
  _sceneItemUButtonBox.pack_start(_selectControllableButton);

  _createControlItemButton.signal_clicked().connect(
      sigc::mem_fun(*this, &SceneWindow::onCreateControlItemButtonPressed));
  _createControlItemButton.set_sensitive(false);
  _sceneItemUButtonBox.pack_start(_createControlItemButton);

  _setEndTimeButton.signal_clicked().connect(
      sigc::mem_fun(*this, &SceneWindow::onSetEndTimeButtonPressed));
  _setEndTimeButton.set_sensitive(false);
  _sceneItemUButtonBox.pack_start(_setEndTimeButton);

  _removeButton.signal_clicked().connect(
      sigc::mem_fun(*this, &SceneWindow::onRemoveButtonPressed));
  _removeButton.set_sensitive(false);
  _sceneItemUButtonBox.pack_start(_removeButton);

  _blackoutButton.signal_clicked().connect([&]() {
    SceneWindow::AddBlackoutItem(theatre::BlackoutOperation::Blackout);
  });
  _blackoutButton.set_sensitive(false);
  _sceneItemUButtonBox.pack_start(_blackoutButton);

  _restoreButton.signal_clicked().connect([&]() {
    SceneWindow::AddBlackoutItem(theatre::BlackoutOperation::Restore);
  });
  _restoreButton.set_sensitive(false);
  _sceneItemUButtonBox.pack_start(_restoreButton);

  _setFadeSpeedButton.signal_clicked().connect([&]() { SetFadeSpeed(); });
  _setFadeSpeedButton.set_sensitive(false);
  _sceneItemUButtonBox.pack_start(_setFadeSpeedButton);

  _sceneItemBox.pack_start(_sceneItemUButtonBox, false, false, 2);

  _startScale.set_inverted(true);
  _startScale.set_draw_value(false);
  _startScale.set_sensitive(false);
  _startScale.signal_value_changed().connect(
      sigc::mem_fun(*this, &SceneWindow::onScalesChanged));
  _scalesBox.pack_start(_startScale);

  _endScale.set_inverted(true);
  _endScale.set_draw_value(false);
  _endScale.set_sensitive(false);
  _endScale.signal_value_changed().connect(
      sigc::mem_fun(*this, &SceneWindow::onScalesChanged));
  _scalesBox.pack_start(_endScale);

  _sceneItemBox.pack_start(_scalesBox);

  _hBox.pack_start(_sceneItemBox, false, false, 2);

  _vBox.pack_start(_hBox);

  add(_vBox);
  show_all_children();

  _timeoutConnection = Glib::signal_timeout().connect(
      sigc::mem_fun(*this, &SceneWindow::onTimeout), 20);

  _updateConnection =
      eventHub.SignalUpdateControllables().connect([&]() { Update(); });
  Update();

  if (Scene *first_scene = FirstScene(_management); first_scene)
    SetSelectedScene(*first_scene);
  else {
    NewScene();
  }
  fillControllablesList();
}

SceneWindow::~SceneWindow() {
  _timeoutConnection.disconnect();
  _updateConnection.disconnect();
}

void SceneWindow::Update() {
  fillControllablesList();
  fillSceneItemList();
  updateAudio();
  UpdateAudioWidgetKeys();
}

void SceneWindow::createSceneItemsList() {
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
      sigc::mem_fun(*this, &SceneWindow::onSelectedSceneItemChanged));
  _sceneItemsListView.set_rubber_banding(true);
  _listScrolledWindow.add(_sceneItemsListView);
  _sceneItemsListView.show();

  _listScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  _hBox.pack_start(_listScrolledWindow, true, true);
  _listScrolledWindow.show();
}

void SceneWindow::createControllablesList() {
  _controllablesListModel = Gtk::ListStore::create(_controllablesListColumns);

  _controllablesComboBox.set_model(_controllablesListModel);
  _controllablesComboBox.pack_start(_controllablesListColumns._text);

  _sceneItemBox.pack_start(_controllablesComboBox, false, false);
  _controllablesComboBox.show();
}

void SceneWindow::fillSceneItemList() {
  _sceneItemsListModel->clear();

  if (_selectedScene != nullptr) {
    std::unique_lock<std::mutex> lock(_management.Mutex());
    const std::multimap<double, std::unique_ptr<theatre::SceneItem>> &items =
        _selectedScene->SceneItems();
    for (const std::pair<const double, std::unique_ptr<theatre::SceneItem>>
             &item : items) {
      Gtk::TreeModel::iterator iter = _sceneItemsListModel->append();
      const Gtk::TreeRow &row = *iter;
      setSceneItemListRow(item.second.get(), row);
    }
    lock.unlock();
    _sceneItemsListView.set_sensitive(true);
  } else {
    _sceneItemsListView.set_sensitive(false);
  }
}

void SceneWindow::setSceneItemListRow(theatre::SceneItem *sceneItem,
                                      const Gtk::TreeModel::Row &row) const {
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

void SceneWindow::updateSelectedSceneItems() {
  Glib::RefPtr<Gtk::TreeSelection> selection =
      _sceneItemsListView.get_selection();
  std::vector<Gtk::TreeModel::Path> pathHandle = selection->get_selected_rows();
  std::lock_guard<std::mutex> lock(_management.Mutex());
  for (Gtk::TreeModel::Path &path : pathHandle) {
    Gtk::TreeModel::iterator iter = _sceneItemsListModel->get_iter(path);
    const Gtk::TreeModel::Row &row = *iter;
    theatre::SceneItem *item = row[_sceneItemsListColumns._item];
    setSceneItemListRow(item, row);
  }
}

void SceneWindow::fillControllablesList() {
  _controllablesListModel->clear();

  if (_selectedScene) {
    std::lock_guard<std::mutex> lock(_management.Mutex());
    if (_latestSelectedControllable) {
      if (_management.Contains(*_latestSelectedControllable)) {
        Gtk::TreeModel::iterator iter = _controllablesListModel->append();
        const Gtk::TreeModel::Row &row = *iter;
        row[_controllablesListColumns._text] =
            _latestSelectedControllable->Name();
        row[_controllablesListColumns._controllable] =
            _latestSelectedControllable;
      } else {
        _latestSelectedControllable = nullptr;
      }
    }
    for (size_t output_index = 0; output_index != _selectedScene->NOutputs();
         ++output_index) {
      std::pair<const glight::theatre::Controllable *, size_t> output =
          _selectedScene->Output(output_index);
      Gtk::TreeModel::iterator iter = _controllablesListModel->append();
      const Gtk::TreeModel::Row &row = *iter;
      row[_controllablesListColumns._text] = output.first->Name();
      row[_controllablesListColumns._controllable] =
          const_cast<glight::theatre::Controllable *>(output.first);
    }
  }
}

void SceneWindow::Rewind() {
  if (_selectedScene != nullptr) {
    StopPlayback();
    std::lock_guard<std::mutex> lock(_management.Mutex());
    _audioWidget.SetPosition(0);
  }
}

void SceneWindow::StopPlayback() {
  start_tb_.set_icon_name("media-playback-start");
  std::lock_guard<std::mutex> lock(_management.Mutex());
  _sourceValue->A().Set(0, 0);
}

void SceneWindow::StartPlayback() {
  if (_selectedScene != nullptr) {
    bool start = !_sourceValue->A().Value();
    if (start) {
      start_tb_.set_icon_name("media-playback-pause");
      std::lock_guard<std::mutex> lock(_management.Mutex());
      _selectedScene->SetStartOffset(_audioWidget.Position());
      _sourceValue->A().Set(theatre::ControlValue::MaxUInt(), 0);
    } else {
      StopPlayback();
    }
  }
}

void SceneWindow::SeekBackward() {
  if (_selectedScene != nullptr) {
    StopPlayback();
    std::lock_guard<std::mutex> lock(_management.Mutex());
    _audioWidget.SetPosition(std::max(3000.0, _audioWidget.Position()) -
                             3000.0);
  }
}

void SceneWindow::SeekForward() {
  if (_selectedScene != nullptr) {
    StopPlayback();
    std::lock_guard<std::mutex> lock(_management.Mutex());
    _audioWidget.SetPosition(_audioWidget.Position() + 3000.0);
  }
}

void SceneWindow::addKey(theatre::KeySceneLevel level) {
  std::unique_lock<std::mutex> lock(_management.Mutex());
  theatre::KeySceneItem *key = _selectedScene->AddKeySceneItem(
      _management.GetOffsetTimeInMS() - _selectedScene->StartTimeInMS());
  key->SetLevel(level);
  lock.unlock();

  fillSceneItemList();
  UpdateAudioWidgetKeys();
}

void SceneWindow::onSelectControllable() {
  InputSelectDialog dialog(_management, _eventHub, false);
  if (dialog.run() == Gtk::RESPONSE_OK) {
    _latestSelectedControllable = dialog.SelectedInput().GetControllable();
    fillControllablesList();
  }
}

void SceneWindow::onCreateControlItemButtonPressed() {
  Gtk::TreeModel::iterator activeControllable =
      _controllablesComboBox.get_active();
  if (activeControllable) {
    _isUpdating = true;

    Glib::RefPtr<Gtk::TreeSelection> selection =
        _sceneItemsListView.get_selection();
    std::vector<Gtk::TreeModel::Path> pathHandle =
        selection->get_selected_rows();
    std::unique_lock<std::mutex> lock(_management.Mutex());
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
      if (_management.HasCycle())
        _selectedScene->Remove(item);
      else {
        if (nextItem != nullptr)
          item->SetDurationInMS(nextItem->OffsetInMS() - item->OffsetInMS());
        else
          item->SetDurationInMS(1000);
      }
    }
    lock.unlock();

    fillSceneItemList();
    UpdateAudioWidgetKeys();
    _isUpdating = false;
    onSelectedSceneItemChanged();
  }
}

void SceneWindow::onSelectedSceneItemChanged() {
  if (!_isUpdating) {
    switch (selectedSceneItemCount()) {
      case 0:
        _createControlItemButton.set_sensitive(false);
        _setEndTimeButton.set_sensitive(false);
        _removeButton.set_sensitive(false);
        _blackoutButton.set_sensitive(false);
        _restoreButton.set_sensitive(false);
        _setFadeSpeedButton.set_sensitive(false);
        _startScale.set_sensitive(false);
        _endScale.set_sensitive(false);
        break;
      case 1: {
        std::unique_lock<std::mutex> lock(_management.Mutex());
        theatre::SceneItem *item = selectedItem();
        const double offset = item->OffsetInMS();
        const bool is_blackout =
            dynamic_cast<theatre::BlackoutSceneItem *>(item);
        if (theatre::ControlSceneItem *csi =
                dynamic_cast<theatre::ControlSceneItem *>(item);
            csi) {
          const unsigned s = csi->StartValue().UInt();
          const unsigned e = csi->EndValue().UInt();
          lock.unlock();
          _startScale.set_value(s);
          _endScale.set_value(e);
        } else {
          lock.unlock();
        }
        _audioWidget.SetPosition(offset);
        _createControlItemButton.set_sensitive(true);
        _setEndTimeButton.set_sensitive(true);
        _removeButton.set_sensitive(true);
        _blackoutButton.set_sensitive(true);
        _restoreButton.set_sensitive(true);
        _setFadeSpeedButton.set_sensitive(is_blackout);
        _startScale.set_sensitive(true);
        _endScale.set_sensitive(true);
      } break;
      default:
        _createControlItemButton.set_sensitive(true);
        _setEndTimeButton.set_sensitive(true);
        _removeButton.set_sensitive(true);
        _blackoutButton.set_sensitive(true);
        _restoreButton.set_sensitive(true);
        _setFadeSpeedButton.set_sensitive(false);
        _startScale.set_sensitive(true);
        _endScale.set_sensitive(true);
        break;
    }
  }
}

void SceneWindow::onSetEndTimeButtonPressed() {
  Glib::RefPtr<Gtk::TreeSelection> selection =
      _sceneItemsListView.get_selection();
  std::vector<Gtk::TreeModel::Path> pathHandle = selection->get_selected_rows();
  std::unique_lock<std::mutex> lock(_management.Mutex());
  for (std::vector<Gtk::TreeModel::Path>::const_iterator pathPtr =
           pathHandle.begin();
       pathPtr != pathHandle.end(); ++pathPtr) {
    theatre::SceneItem *selItem = (*_sceneItemsListModel->get_iter(
        *pathPtr))[_sceneItemsListColumns._item];
    theatre::SceneItem *nextItem = nullptr;
    std::vector<Gtk::TreeModel::Path>::const_iterator nextPtr = pathPtr;
    ++nextPtr;
    if (nextPtr != pathHandle.end()) {
      nextItem = (*_sceneItemsListModel->get_iter(
          *nextPtr))[_sceneItemsListColumns._item];
      selItem->SetDurationInMS(nextItem->OffsetInMS() - selItem->OffsetInMS());
    }
  }
  lock.unlock();

  UpdateAudioWidgetKeys();
  updateSelectedSceneItems();
}

void SceneWindow::onRemoveButtonPressed() {
  _isUpdating = true;
  Glib::RefPtr<Gtk::TreeSelection> selection =
      _sceneItemsListView.get_selection();
  std::vector<Gtk::TreeModel::Path> pathHandle = selection->get_selected_rows();
  std::unique_lock<std::mutex> lock(_management.Mutex());
  for (const Gtk::TreeModel::Path &path : pathHandle) {
    theatre::SceneItem *item =
        (*_sceneItemsListModel->get_iter(path))[_sceneItemsListColumns._item];
    _selectedScene->Remove(item);
  }
  lock.unlock();
  fillSceneItemList();
  UpdateAudioWidgetKeys();
  _isUpdating = false;
  onSelectedSceneItemChanged();
}

bool SceneWindow::HandleKeyDown(char key) {
  using theatre::KeySceneLevel;
  switch (key) {
    case '=':
      StartPlayback();
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

void SceneWindow::SetSelectedScene(theatre::Scene &scene) {
  _selectedScene = &scene;
  _sourceValue = _management.GetSourceValue(scene, 0);
  updateAudio();
  UpdateAudioWidgetKeys();
  set_sensitive(true);
}

void SceneWindow::SetNoSelectedScene() {
  set_sensitive(false);
  _selectedScene = nullptr;
  _sourceValue = nullptr;
  _audioWidget.SetNoScene();
  updateAudio();
}

void SceneWindow::onScalesChanged() {
  if (!_isUpdating) {
    _isUpdating = true;
    Glib::RefPtr<Gtk::TreeSelection> selection =
        _sceneItemsListView.get_selection();
    std::vector<Gtk::TreeModel::Path> pathHandle =
        selection->get_selected_rows();
    std::unique_lock<std::mutex> lock(_management.Mutex());
    for (const Gtk::TreeModel::Path &path : pathHandle) {
      theatre::SceneItem *item =
          (*_sceneItemsListModel->get_iter(path))[_sceneItemsListColumns._item];
      theatre::ControlSceneItem *csi =
          dynamic_cast<theatre::ControlSceneItem *>(item);
      if (csi != nullptr) {
        csi->StartValue().Set(static_cast<unsigned>(_startScale.get_value()));
        csi->EndValue().Set(static_cast<unsigned>(_endScale.get_value()));
      }
    }
    lock.unlock();
    updateSelectedSceneItems();
    _isUpdating = false;
  }
}

void SceneWindow::ChangeAudio() {
  if (_selectedScene) {
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
}

bool SceneWindow::onTimeout() {
  std::unique_lock<std::mutex> lock(_management.Mutex());
  if (_selectedScene != nullptr && _selectedScene->IsPlaying()) {
    double pos =
        _management.GetOffsetTimeInMS() - _selectedScene->StartTimeInMS();
    lock.unlock();
    _audioWidget.SetPosition(pos);
  }
  return true;
}

void SceneWindow::onAudioWidgetClicked(double timeInMS) {
  if (set_start_tb_.get_active()) {
    _isUpdating = true;
    Glib::RefPtr<Gtk::TreeSelection> selection =
        _sceneItemsListView.get_selection();
    std::vector<Gtk::TreeModel::Path> pathHandle =
        selection->get_selected_rows();
    std::unique_lock<std::mutex> lock(_management.Mutex());
    for (const Gtk::TreeModel::Path &path : pathHandle) {
      theatre::SceneItem *item =
          (*_sceneItemsListModel->get_iter(path))[_sceneItemsListColumns._item];
      _selectedScene->ChangeSceneItemStartTime(item, timeInMS);
    }
    lock.unlock();
    _isUpdating = false;
    updateSelectedSceneItems();
    UpdateAudioWidgetKeys();
  }

  else if (set_end_tb_.get_active()) {
    _isUpdating = true;
    Glib::RefPtr<Gtk::TreeSelection> selection =
        _sceneItemsListView.get_selection();
    std::vector<Gtk::TreeModel::Path> pathHandle =
        selection->get_selected_rows();
    std::unique_lock<std::mutex> lock(_management.Mutex());
    for (const Gtk::TreeModel::Path &path : pathHandle) {
      theatre::SceneItem *item =
          (*_sceneItemsListModel->get_iter(path))[_sceneItemsListColumns._item];
      item->SetDurationInMS(timeInMS - item->OffsetInMS());
    }
    lock.unlock();
    _isUpdating = false;
    updateSelectedSceneItems();
    UpdateAudioWidgetKeys();
  }

  else if (add_key_tb_.get_active()) {
    _isUpdating = true;

    std::unique_lock<std::mutex> lock(_management.Mutex());
    theatre::KeySceneItem *item = _selectedScene->AddKeySceneItem(timeInMS);
    item->SetLevel(theatre::KeySceneLevel::Key);
    lock.unlock();

    fillSceneItemList();
    UpdateAudioWidgetKeys();
    _isUpdating = false;
    onSelectedSceneItemChanged();
  } else if (add_item_tb_.get_active()) {
    Gtk::TreeModel::iterator activeControllable =
        _controllablesComboBox.get_active();
    if (activeControllable) {
      _isUpdating = true;

      std::unique_lock<std::mutex> lock(_management.Mutex());
      theatre::ControlSceneItem *item = _selectedScene->AddControlSceneItem(
          timeInMS,
          *(*activeControllable)[_controllablesListColumns._controllable], 0);
      if (_management.HasCycle())
        _selectedScene->Remove(item);
      else
        item->SetDurationInMS(1000);
      lock.unlock();

      fillSceneItemList();
      UpdateAudioWidgetKeys();
      _isUpdating = false;
      onSelectedSceneItemChanged();
    }
  } else
    _audioWidget.SetPosition(timeInMS);
}

void SceneWindow::updateAudio() {
  std::unique_lock<std::mutex> lock(_management.Mutex());
  if (_selectedScene != nullptr && _selectedScene->AudioFile() != _audioFile) {
    _audioFile = _selectedScene->AudioFile();
    lock.unlock();
    std::string short_title;
    if (_audioFile.size() > 20)
      short_title = "..." + _audioFile.substr(_audioFile.size() - 17);
    else
      short_title = _audioFile;
    _audioLabel.set_text(std::string("Audio file: ") + short_title);
    if (_audioFile.empty())
      _audioWidget.ClearAudioData();
    else
      try {
        system::FlacDecoder decoder(_audioFile);
        decoder.Start();
        _audioWidget.SetAudioData(decoder);
      } catch (std::exception &e) {
        std::cout << e.what() << '\n';
      }
  }
}

bool SceneWindow::NewScene() {
  Gtk::MessageDialog dialog(*this, "Name fader setup", false,
                            Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
  Gtk::Entry entry;
  dialog.get_vbox()->pack_start(entry, Gtk::PACK_SHRINK);
  dialog.get_vbox()->show_all_children();
  dialog.set_secondary_text("Name of new scene:");
  int result = dialog.run();
  if (result == Gtk::RESPONSE_OK) {
    glight::theatre::Scene &scene = _management.AddScene(true);
    if (!scene.Parent().GetChildIfExists(entry.get_text())) {
      scene.SetName(entry.get_text());
    }
    _sourceValue = &_management.AddSourceValue(scene, 0);
    SetSelectedScene(scene);
    _eventHub.EmitUpdate();
    return true;
  } else {
    return false;
  }
}

void SceneWindow::LoadScene() {
  dialogs::SceneSelect dialog(_management, _eventHub);
  dialog.SetSelection(*_selectedScene);
  if (dialog.run() == Gtk::RESPONSE_OK) {
    Scene &scene = *dialog.GetSelection();
    _sourceValue = &_management.AddSourceValue(scene, 0);
    SetSelectedScene(scene);
  }
}

void SceneWindow::UpdateAudioWidgetKeys() {
  std::lock_guard<std::mutex> lock(_management.Mutex());
  if (_selectedScene)
    _audioWidget.SetScene(*_selectedScene);
  else
    _audioWidget.SetNoScene();
}

void SceneWindow::AddBlackoutItem(theatre::BlackoutOperation operation) {
  Glib::RefPtr<Gtk::TreeSelection> selection =
      _sceneItemsListView.get_selection();
  Gtk::TreeModel::Path path = selection->get_selected_rows().front();

  std::unique_lock<std::mutex> lock(_management.Mutex());
  theatre::SceneItem *selected_item =
      (*_sceneItemsListModel->get_iter(path))[_sceneItemsListColumns._item];
  theatre::BlackoutSceneItem &new_item =
      _selectedScene->AddBlackoutItem(selected_item->OffsetInMS());
  new_item.SetFadeSpeed(1.0);
  new_item.SetOperation(operation);
  lock.unlock();

  fillSceneItemList();
  UpdateAudioWidgetKeys();
}

void SceneWindow::SetFadeSpeed() {
  Glib::RefPtr<Gtk::TreeSelection> selection =
      _sceneItemsListView.get_selection();
  Gtk::TreeModel::Path path = selection->get_selected_rows().front();

  std::unique_lock<std::mutex> lock(_management.Mutex());
  theatre::SceneItem *selected_item =
      (*_sceneItemsListModel->get_iter(path))[_sceneItemsListColumns._item];
  theatre::BlackoutSceneItem *blackout =
      dynamic_cast<theatre::BlackoutSceneItem *>(selected_item);
  if (blackout) {
    const double fade_speed = blackout->FadeSpeed();
    lock.unlock();

    std::stringstream fade_speed_str;
    fade_speed_str << fade_speed;
    StringInputDialog dialog("Fade speed",
                             "Enter new fade speed:", fade_speed_str.str());
    dialog.run();

    lock.lock();
    selected_item =
        (*_sceneItemsListModel->get_iter(path))[_sceneItemsListColumns._item];
    blackout = dynamic_cast<theatre::BlackoutSceneItem *>(selected_item);
    if (blackout) {
      blackout->SetFadeSpeed(std::atof(dialog.Value().c_str()));
    }
    lock.unlock();
  }
}

}  // namespace glight::gui
