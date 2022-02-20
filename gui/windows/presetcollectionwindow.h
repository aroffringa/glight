#ifndef PRESET_COLLECTION_WINDOW_H
#define PRESET_COLLECTION_WINDOW_H

#include "propertieswindow.h"

#include "../recursionlock.h"

#include "../components/inputselectwidget.h"

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

/**
        @author Andre Offringa
*/
class PresetCollectionWindow : public PropertiesWindow {
 public:
  PresetCollectionWindow(PresetCollection &presetCollection,
                         class Management &management,
                         class EventTransmitter &eventHub);
  ~PresetCollectionWindow();

  class FolderObject &GetObject() final override;
  class PresetCollection &GetPresetCollection() {
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

  void onChangeManagement(class Management &management) {
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

  PresetCollection *_presetCollection;
  Management *_management;
  EventTransmitter &_eventHub;
  sigc::connection _changeManagementConnection, _updateControllablesConnection;
};

#endif
