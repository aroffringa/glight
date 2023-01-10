#ifndef GUI_SCENEFRAME_H_
#define GUI_SCENEFRAME_H_

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/combobox.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>

#include "../components/audiowidget.h"

#include "../../theatre/forwards.h"
#include "../../theatre/keysceneitem.h"

#include "../nameframe.h"

namespace glight::gui {

class EventTransmitter;
class ShowWindow;

class SceneWindow : public Gtk::Window {
 public:
  SceneWindow(theatre::Management &management, ShowWindow &parentWindow,
              EventTransmitter &eventHub);
  ~SceneWindow();

  bool HandleKeyDown(char key);
  void SetSelectedScene(theatre::Scene &scene) {
    _selectedScene = &scene;
    _audioWidget.SetScene(scene);
    set_sensitive(true);
  }
  void SetNoSelectedScene() {
    set_sensitive(false);
    _audioWidget.SetNoScene();
    _selectedScene = nullptr;
  }

 private:
  theatre::Management &_management;
  EventTransmitter &_eventHub;

  struct SceneItemsListColumns : public Gtk::TreeModelColumnRecord {
    SceneItemsListColumns() {
      add(_startTime);
      add(_endTime);
      add(_startValue), add(_endValue);
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
  void addTool(Gtk::ToolButton &tool, const char *label, const char *tooltip,
               const char *icon, const SigType &sig) {
    tool.set_label(label);
    tool.set_tooltip_text(tooltip);
    tool.set_icon_name(icon);
    tool.signal_clicked().connect(sig);
    _toolbar.append(tool);
  }
  Gtk::ToolButton new_scene_tb_;
  Gtk::Toolbar _toolbar;
  AudioWidget _audioWidget;
  Gtk::Label _clickIsLabel;
  Gtk::RadioButton _clickIsSelectButton, _clickIsSetStartButton,
      _clickIsSetEndButton, _clickIsAddKeyButton, _clickIsAddItemButton;
  Gtk::TreeView _sceneItemsListView;
  Gtk::ComboBox _controllables1ComboBox, _controllables2ComboBox;
  Glib::RefPtr<Gtk::ListStore> _sceneItemsListModel, _controllablesListModel;
  Gtk::VBox _vBox;
  Gtk::HBox _hBox;
  Gtk::HBox _audioBox;
  Gtk::Label _audioLabel;
  Gtk::Button _changeAudioButton;
  Gtk::ScrolledWindow _listScrolledWindow;
  Gtk::VBox _sceneItemBox;
  Gtk::HBox _scalesBox;
  Gtk::VButtonBox _sceneItemUButtonBox;

  Gtk::Button _startButton;
  Gtk::Button _startSelectionButton;
  Gtk::Button _stopButton;
  Gtk::Button _key1Button;
  Gtk::Button _createControlItemButton;
  Gtk::Button _setEndTimeButton;
  Gtk::Button _removeButton;
  Gtk::Button _createTransitionItemButton;
  Gtk::VScale _startScale, _endScale;

  sigc::connection _updateConnection;
  sigc::connection _timeoutConnection;

  std::string _audioFile;

  NameFrame _nameFrame;

  theatre::Scene *_selectedScene;
  bool _isUpdating;

  bool NewScene();
  void Update();

  void createSceneItemsList();
  void createControllablesList1();
  void createControllablesList2();
  void fillSceneItemList();
  void setSceneItemListRow(theatre::SceneItem *sceneItem,
                           Gtk::TreeModel::Row row);
  void updateSelectedSceneItems();
  void fillControllablesList();
  void addKey(theatre::KeySceneLevel level);

  void onChangeAudioButtonPressed();
  void onStartButtonPressed();
  void onStartSelectionButtonPressed();
  void onStopButtonPressed();
  void onKey1ButtonPressed();
  void onCreateControlItemButtonPressed();
  void onSelectedSceneItemChanged();
  void onSetEndTimeButtonPressed();
  void onRemoveButtonPressed();
  void onScalesChanged();
  bool onTimeout();
  void onAudioWidgetClicked(double timeInMS);

  void updateAudio();
  void updateAudioWidgetKeys();

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
