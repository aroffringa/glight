#include "scenewindow.h"

#include <cmath>
#include <iostream>
#include <sstream>

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>

#include <glibmm/main.h>

#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/sourcevalue.h"

#include "theatre/scenes/scene.h"

#include "gui/eventtransmitter.h"
#include "gui/instance.h"

#include "gui/dialogs/inputselectdialog.h"
#include "gui/dialogs/stringinputdialog.h"
#include "gui/dialogs/sceneselect.h"

using glight::system::TrackablePtr;
using glight::theatre::Controllable;
using glight::theatre::Scene;

namespace {
Scene *FirstScene(glight::theatre::Management &management) {
  for (const TrackablePtr<Controllable> &controllable :
       management.Controllables()) {
    if (Scene *scene = dynamic_cast<Scene *>(controllable.Get()); scene) {
      return scene;
    }
  }
  return nullptr;
}

}  // namespace

namespace glight::gui {

SceneWindow::SceneWindow(MainWindow &parentWindow)
    : _management(Instance::Management()),
      _audioLabel("Audio file: -"),
      _sceneItemUButtonBox(Gtk::Orientation::VERTICAL),
      _selectControllableButton("Select..."),
      _createControlItemButton("Add"),
      _setEndTimeButton("Set end time"),
      _removeButton("Remove"),
      _blackoutButton("Black-out"),
      _restoreButton("Restore"),
      _setFadeSpeedButton("Fade speed"),
      _startScale(
          Gtk::Adjustment::create(0, 0, theatre::ControlValue::MaxUInt() + 1,
                                  theatre::ControlValue::MaxUInt() / 100.0),
          Gtk::Orientation::VERTICAL),
      _endScale(
          Gtk::Adjustment::create(0, 0, theatre::ControlValue::MaxUInt() + 1,
                                  theatre::ControlValue::MaxUInt() / 100.0),
          Gtk::Orientation::VERTICAL),
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

  addTool(move_cursor_tb_, "Move cursor",
          "Clicking the audio will move the cursor", "go-jump", [&]() {});
  addTool(set_start_tb_, "Set start time",
          "Clicking the audio will change the start time of the selection",
          "go-first", [&]() {});
  set_start_tb_.set_group(move_cursor_tb_);
  addTool(set_end_tb_, "Set end time",
          "Clicking the audio will change the end time of the selection",
          "go-last", [&]() {});
  set_end_tb_.set_group(move_cursor_tb_);
  addTool(add_key_tb_, "Add key", "Clicking the audio will add a key",
          "starred", [&]() {});
  add_key_tb_.set_group(move_cursor_tb_);
  addTool(add_item_tb_, "Add item", "Clicking the audio will add an item",
          "list-add", [&]() {});
  add_item_tb_.set_group(move_cursor_tb_);

  _vBox.append(_toolbar);

  _audioWidget.SignalClicked().connect(
      sigc::mem_fun(*this, &SceneWindow::onAudioWidgetClicked));
  _vBox.append(_audioWidget);

  _audioBox.append(_audioLabel);

  _vBox.append(_audioBox);

  createSceneItemsList();

  createControllablesList();

  _sceneItemUButtonBox.set_homogeneous(true);

  _selectControllableButton.signal_clicked().connect(
      sigc::mem_fun(*this, &SceneWindow::onSelectControllable));
  _sceneItemUButtonBox.append(_selectControllableButton);

  _createControlItemButton.set_image_from_icon_name("list-add");
  _createControlItemButton.signal_clicked().connect(
      sigc::mem_fun(*this, &SceneWindow::onCreateControlItemButtonPressed));
  _createControlItemButton.set_sensitive(false);
  _sceneItemUButtonBox.append(_createControlItemButton);

  _setEndTimeButton.signal_clicked().connect(
      sigc::mem_fun(*this, &SceneWindow::onSetEndTimeButtonPressed));
  _setEndTimeButton.set_sensitive(false);
  _sceneItemUButtonBox.append(_setEndTimeButton);

  _removeButton.set_image_from_icon_name("edit-delete");
  _removeButton.signal_clicked().connect(
      sigc::mem_fun(*this, &SceneWindow::onRemoveButtonPressed));
  _removeButton.set_sensitive(false);
  _sceneItemUButtonBox.append(_removeButton);

  _blackoutButton.signal_clicked().connect([&]() {
    SceneWindow::AddBlackoutItem(theatre::BlackoutOperation::Blackout);
  });
  _blackoutButton.set_sensitive(false);
  _sceneItemUButtonBox.append(_blackoutButton);

  _restoreButton.signal_clicked().connect([&]() {
    SceneWindow::AddBlackoutItem(theatre::BlackoutOperation::Restore);
  });
  _restoreButton.set_sensitive(false);
  _sceneItemUButtonBox.append(_restoreButton);

  _setFadeSpeedButton.signal_clicked().connect([&]() { SetFadeSpeed(); });
  _setFadeSpeedButton.set_sensitive(false);
  _sceneItemUButtonBox.append(_setFadeSpeedButton);

  _sceneItemBox.append(_sceneItemUButtonBox);

  _startScale.set_inverted(true);
  _startScale.set_draw_value(false);
  _startScale.set_sensitive(false);
  _startScale.signal_value_changed().connect(
      sigc::mem_fun(*this, &SceneWindow::onScalesChanged));
  _scalesBox.append(_startScale);

  _endScale.set_inverted(true);
  _endScale.set_draw_value(false);
  _endScale.set_sensitive(false);
  _endScale.signal_value_changed().connect(
      sigc::mem_fun(*this, &SceneWindow::onScalesChanged));
  _scalesBox.append(_endScale);

  _sceneItemBox.append(_scalesBox);

  _hBox.append(_sceneItemBox);

  _vBox.append(_hBox);

  set_child(_vBox);

  _timeoutConnection = Glib::signal_timeout().connect(
      sigc::mem_fun(*this, &SceneWindow::onTimeout), 20);

  _updateConnection = Instance::Events().SignalUpdateControllables().connect(
      [&]() { Update(); });
  Update();

  if (Scene *first_scene = FirstScene(_management); first_scene)
    SetSelectedScene(*first_scene);
  else {
    NewScene();
  }
  Update();
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
  _sceneItemsListView.get_selection()->set_mode(Gtk::SelectionMode::MULTIPLE);
  _sceneItemsListView.get_selection()->signal_changed().connect(
      sigc::mem_fun(*this, &SceneWindow::onSelectedSceneItemChanged));
  _sceneItemsListView.set_rubber_banding(true);
  _listScrolledWindow.set_child(_sceneItemsListView);
  _sceneItemsListView.show();

  _listScrolledWindow.set_policy(Gtk::PolicyType::NEVER,
                                 Gtk::PolicyType::AUTOMATIC);
  _hBox.append(_listScrolledWindow);
  _listScrolledWindow.show();
}

void SceneWindow::createControllablesList() {
  _controllablesListModel = Gtk::ListStore::create(_controllablesListColumns);

  _controllablesComboBox.set_model(_controllablesListModel);
  _controllablesComboBox.pack_start(_controllablesListColumns._text);

  _sceneItemBox.append(_controllablesComboBox);
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
      Gtk::TreeRow &row = *iter;
      setSceneItemListRow(item.second.get(), row);
    }
    lock.unlock();
    _sceneItemsListView.set_sensitive(true);
  } else {
    _sceneItemsListView.set_sensitive(false);
  }
}

void SceneWindow::setSceneItemListRow(theatre::SceneItem *sceneItem,
                                      Gtk::TreeModel::Row &row) const {
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
    Gtk::TreeModel::Row &row = *iter;
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
        Gtk::TreeModel::Row &row = *iter;
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
      Gtk::TreeModel::Row &row = *iter;
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
  dialog_ = std::make_unique<InputSelectDialog>(false);
  dialog_->signal_response().connect([this](int response) {
    if (response == Gtk::ResponseType::OK) {
      InputSelectDialog &dialog = static_cast<InputSelectDialog &>(*dialog_);
      _latestSelectedControllable = dialog.SelectedInput().GetControllable();
      fillControllablesList();
    }
    dialog_.reset();
  });
  dialog_->show();
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
  Update();
  set_sensitive(true);
}

void SceneWindow::SetNoSelectedScene() {
  set_sensitive(false);
  _selectedScene = nullptr;
  _sourceValue = nullptr;
  _audioWidget.SetNoScene();
  Update();
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
    dialog_ = std::make_unique<Gtk::FileChooserDialog>(
        "Open audio file", Gtk::FileChooser::Action::OPEN);
    Gtk::FileChooserDialog &dialog =
        static_cast<Gtk::FileChooserDialog &>(*dialog_);
    dialog.add_button("Cancel", Gtk::ResponseType::CANCEL);
    dialog.add_button("Open", Gtk::ResponseType::OK);

    Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
    filter->set_name("Flac audio file");
    filter->add_pattern("*.flac");
    filter->add_mime_type("audio/flac");
    dialog.add_filter(filter);
    dialog.signal_response().connect([this](int response) {
      if (response == Gtk::ResponseType::OK) {
        Gtk::FileChooserDialog &dialog =
            static_cast<Gtk::FileChooserDialog &>(*dialog_);
        _selectedScene->SetAudioFile(dialog.get_file()->get_path());
        updateAudio();
      }
      dialog_.reset();
    });
    dialog.show();
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

void SceneWindow::NewScene() {
  dialog_ = std::make_unique<Gtk::MessageDialog>(
      *this, "Name fader setup", false, Gtk::MessageType::QUESTION,
      Gtk::ButtonsType::OK_CANCEL);
  Gtk::MessageDialog &dialog = static_cast<Gtk::MessageDialog &>(*dialog_);
  dialog_entry_ = Gtk::Entry();
  dialog.get_message_area()->append(dialog_entry_);
  dialog.set_secondary_text("Name of new scene:");
  dialog.signal_response().connect([this](int response) {
    if (response == Gtk::ResponseType::OK) {
      theatre::Scene &scene =
          static_cast<theatre::Scene &>(*_management.AddScene(true));
      if (!scene.Parent().GetChildIfExists(
              std::string(dialog_entry_.get_text()))) {
        scene.SetName(dialog_entry_.get_text());
      }
      _sourceValue = &_management.AddSourceValue(scene, 0);
      SetSelectedScene(scene);
      Instance::Events().EmitUpdate();
    }
    dialog_.reset();
  });
  dialog.show();
}

void SceneWindow::LoadScene() {
  dialog_ = std::make_unique<dialogs::SceneSelect>();
  dialogs::SceneSelect &dialog = static_cast<dialogs::SceneSelect &>(*dialog_);
  dialog.SetSelection(*_selectedScene);
  dialog.signal_response().connect([this](int response) {
    if (response == Gtk::ResponseType::OK) {
      dialogs::SceneSelect &dialog =
          static_cast<dialogs::SceneSelect &>(*dialog_);
      Scene &scene = *dialog.GetSelection();
      _sourceValue = &_management.AddSourceValue(scene, 0);
      SetSelectedScene(scene);
    }
    dialog_.reset();
  });
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
  std::unique_lock<std::mutex> lock(_management.Mutex());
  Glib::RefPtr<Gtk::TreeSelection> selection =
      _sceneItemsListView.get_selection();
  Gtk::TreeModel::Path path = selection->get_selected_rows().front();
  theatre::SceneItem *selected_item =
      (*_sceneItemsListModel->get_iter(path))[_sceneItemsListColumns._item];
  theatre::BlackoutSceneItem *blackout =
      dynamic_cast<theatre::BlackoutSceneItem *>(selected_item);
  if (blackout) {
    const double fade_speed = blackout->FadeSpeed();
    lock.unlock();

    std::stringstream fade_speed_str;
    fade_speed_str << fade_speed;
    dialog_ = std::make_unique<StringInputDialog>(
        "Fade speed", "Enter new fade speed:", fade_speed_str.str());
    dialog_->signal_response().connect([this](int response) {
      Glib::RefPtr<Gtk::TreeSelection> selection =
          _sceneItemsListView.get_selection();
      Gtk::TreeModel::Path path = selection->get_selected_rows().front();
      StringInputDialog &dialog = static_cast<StringInputDialog &>(*dialog_);
      theatre::SceneItem *selected_item =
          (*_sceneItemsListModel->get_iter(path))[_sceneItemsListColumns._item];

      std::lock_guard<std::mutex> lock(_management.Mutex());
      theatre::BlackoutSceneItem *blackout =
          dynamic_cast<theatre::BlackoutSceneItem *>(selected_item);
      if (blackout) {
        blackout->SetFadeSpeed(std::atof(dialog.Value().c_str()));
      }
    });
    dialog_->show();
  }
}

}  // namespace glight::gui
