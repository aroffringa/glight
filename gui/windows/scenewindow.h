#ifndef GUI_SCENEFRAME_H_
#define GUI_SCENEFRAME_H_

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/dialog.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scale.h>
#include <gtkmm/separator.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

#include "gui/components/audiowidget.h"

#include "theatre/forwards.h"

#include "theatre/scenes/blackoutsceneitem.h"
#include "theatre/scenes/keysceneitem.h"

namespace glight::gui {

class EventTransmitter;
class MainWindow;

class SceneWindow : public Gtk::Window {
 public:
  SceneWindow(theatre::Management &management, MainWindow &parentWindow,
              EventTransmitter &eventHub);
  ~SceneWindow();

  bool HandleKeyDown(char key);
  void SetSelectedScene(theatre::Scene &scene);
  void SetNoSelectedScene();

 private:
  theatre::Management &_management;
  EventTransmitter &_eventHub;

  struct SceneItemsListColumns : public Gtk::TreeModelColumnRecord {
    SceneItemsListColumns() {
      add(_startTime);
      add(_endTime);
      add(_startValue);
      add(_endValue);
      add(_description);
      add(_item);
    }

    Gtk::TreeModelColumn<Glib::ustring> _startTime;
    Gtk::TreeModelColumn<Glib::ustring> _endTime;
    Gtk::TreeModelColumn<Glib::ustring> _startValue;
    Gtk::TreeModelColumn<Glib::ustring> _endValue;
    Gtk::TreeModelColumn<Glib::ustring> _description;
    Gtk::TreeModelColumn<theatre::SceneItem *> _item;
  } _sceneItemsListColumns;

  struct ControllablesListColumns : public Gtk::TreeModelColumnRecord {
    ControllablesListColumns() {
      add(_text);
      add(_controllable);
    }

    Gtk::TreeModelColumn<Glib::ustring> _text;
    Gtk::TreeModelColumn<theatre::Controllable *> _controllable;
  } _controllablesListColumns;

  template <typename SigType>
  void addTool(Gtk::Button &tool, const char *label, const char *tooltip,
               const char *icon, const SigType &sig) {
    tool.set_label(label);
    tool.set_tooltip_text(tooltip);
    tool.set_icon_name(icon);
    tool.signal_clicked().connect(sig);
    _toolbar.append(tool);
  }
  Gtk::Button new_scene_tb_;
  Gtk::Button load_scene_tb_;
  Gtk::Separator separator1_;
  Gtk::Button rewind_tb_;
  Gtk::Button pause_tb_;
  Gtk::Button start_tb_;
  Gtk::Button seek_backward_tb_;
  Gtk::Button seek_forward_tb_;
  Gtk::Button change_audio_tb_;
  Gtk::Separator separator2_;
  Gtk::ToggleButton move_cursor_tb_;
  Gtk::ToggleButton set_start_tb_;
  Gtk::ToggleButton set_end_tb_;
  Gtk::ToggleButton add_key_tb_;
  Gtk::ToggleButton add_item_tb_;

  Gtk::Box _toolbar;
  AudioWidget _audioWidget;
  Gtk::TreeView _sceneItemsListView;
  Glib::RefPtr<Gtk::ListStore> _sceneItemsListModel;
  Gtk::ComboBox _controllablesComboBox;
  Glib::RefPtr<Gtk::ListStore> _controllablesListModel;
  theatre::Controllable *_latestSelectedControllable = nullptr;
  Gtk::Box _vBox{Gtk::Orientation::VERTICAL};
  Gtk::Box _hBox;
  Gtk::Box _audioBox;
  Gtk::Label _audioLabel;
  Gtk::ScrolledWindow _listScrolledWindow;
  Gtk::Box _sceneItemBox{Gtk::Orientation::VERTICAL};
  Gtk::Box _scalesBox;
  Gtk::Box _sceneItemUButtonBox;

  Gtk::Button _selectControllableButton;
  Gtk::Button _createControlItemButton;
  Gtk::Button _setEndTimeButton;
  Gtk::Button _removeButton;
  Gtk::Button _blackoutButton;
  Gtk::Button _restoreButton;
  Gtk::Button _setFadeSpeedButton;
  Gtk::Scale _startScale, _endScale;
  std::unique_ptr<Gtk::Dialog> dialog_;
  Gtk::Entry dialog_entry_;

  sigc::connection _updateConnection;
  sigc::connection _timeoutConnection;

  std::string _audioFile;

  theatre::Scene *_selectedScene;
  theatre::SourceValue *_sourceValue;
  bool _isUpdating;

  void NewScene();
  void LoadScene();
  void Update();

  void createSceneItemsList();
  void createControllablesList();
  void fillSceneItemList();
  void setSceneItemListRow(theatre::SceneItem *sceneItem,
                           Gtk::TreeModel::Row &row) const;
  void updateSelectedSceneItems();
  void fillControllablesList();
  void addKey(theatre::KeySceneLevel level);

  void Rewind();
  void StopPlayback();
  void StartPlayback();
  void SeekBackward();
  void SeekForward();
  void ChangeAudio();

  void onSelectControllable();
  void onCreateControlItemButtonPressed();
  void onSelectedSceneItemChanged();
  void onSetEndTimeButtonPressed();
  void onRemoveButtonPressed();
  void AddBlackoutItem(theatre::BlackoutOperation operation);
  void onScalesChanged();
  bool onTimeout();
  void onAudioWidgetClicked(double timeInMS);

  void updateAudio();
  void UpdateAudioWidgetKeys();
  void SetFadeSpeed();

  size_t selectedSceneItemCount() const {
    return _sceneItemsListView.get_selection()->count_selected_rows();
  }

  theatre::SceneItem *selectedItem() {
    if (selectedSceneItemCount() == 1) {
      Glib::RefPtr<Gtk::TreeSelection> selection =
          _sceneItemsListView.get_selection();
      Gtk::TreeModel::Path selected = *selection->get_selected_rows().begin();
      return (*_sceneItemsListModel->get_iter(
          selected))[_sceneItemsListColumns._item];
    } else {
      return nullptr;
    }
  }
};

}  // namespace glight::gui

#endif
