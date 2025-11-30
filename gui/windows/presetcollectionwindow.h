#ifndef GUI_PRESET_COLLECTION_WINDOW_H_
#define GUI_PRESET_COLLECTION_WINDOW_H_

#include "propertieswindow.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>

#include "gui/recursionlock.h"
#include <sigc++/scoped_connection.h>

#include "gui/components/inputselectwidget.h"

#include "theatre/forwards.h"

namespace glight::gui {

class EventTransmitter;

class PresetCollectionWindow : public PropertiesWindow {
 public:
  PresetCollectionWindow(theatre::PresetCollection &presetCollection);

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
  void onUpdateControllables();
  void setPresetSensitive(bool sensitive);
  bool selectedPresetIndex(size_t &index);
  void selectPreset(size_t index);

  Gtk::Box _topBox;
  InputSelectWidget _inputSelector;

  Gtk::Box _buttonBox{Gtk::Orientation::VERTICAL};
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
  sigc::scoped_connection update_connection_;
};

}  // namespace glight::gui

#endif
