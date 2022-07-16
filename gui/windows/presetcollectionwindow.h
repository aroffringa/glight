#ifndef GUI_PRESET_COLLECTION_WINDOW_H_
#define GUI_PRESET_COLLECTION_WINDOW_H_

#include "propertieswindow.h"

#include "../recursionlock.h"

#include "../components/inputselectwidget.h"

#include "../../theatre/forwards.h"
#include "../../theatre/presetcollection.h"
#include "../../theatre/transition.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/frame.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/separator.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

namespace glight::gui {

class EventTransmitter;

class PresetCollectionWindow : public PropertiesWindow {
 public:
  PresetCollectionWindow(theatre::PresetCollection &presetCollection,
                         theatre::Management &management,
                         EventTransmitter &eventHub);
  ~PresetCollectionWindow();

  theatre::FolderObject &GetObject() final override;
  theatre::PresetCollection &GetPresetCollection() {
    return *_presetCollection;
  }

 private:
  void onInputSelectionChanged();
  void onSelectedPresetChanged();
  void load();
  void fillPresetsList();
  void loadPreset(size_t index);
  void onAddPreset();
  void onRemovePreset();
  void onControlValueChanged();

  void onChangeManagement(theatre::Management &management) {
    _management = &management;
  }
  void onUpdateControllables();
  void setPresetSensitive(bool sensitive);
  bool selectedPresetIndex(size_t &index);
  void selectPreset(size_t index);

  Gtk::HBox _topBox;
  InputSelectWidget _inputSelector;

  Gtk::VBox _buttonBox;
  Gtk::Button _addPresetButton;
  Gtk::Button _removePresetButton;

  Gtk::Grid _grid;
  Gtk::TreeView _presetsView;
  Glib::RefPtr<Gtk::ListStore> _presetsStore;
  struct PresetListColumns : public Gtk::TreeModelColumnRecord {
    PresetListColumns() {
      add(_control);
      add(_value);
      add(_presetIndex);
    }

    Gtk::TreeModelColumn<Glib::ustring> _control;
    Gtk::TreeModelColumn<Glib::ustring> _value;
    Gtk::TreeModelColumn<size_t> _presetIndex;
  } _presetListColumns;
  Gtk::ScrolledWindow _presetsScrolledWindow;

  Gtk::Label _controlValueLabel;
  Gtk::Entry _controlValueEntry;

  RecursionLock _recursionLock;

  theatre::PresetCollection *_presetCollection;
  theatre::Management *_management;
  EventTransmitter &_eventHub;
  sigc::connection _changeManagementConnection, _updateControllablesConnection;
};

}  // namespace glight::gui

#endif
