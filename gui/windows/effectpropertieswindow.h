#ifndef GUI_EFFECT_PROPERTIES_WINDOW_H_
#define GUI_EFFECT_PROPERTIES_WINDOW_H_

#include "propertieswindow.h"

#include "../../theatre/forwards.h"

#include "../components/propertiesbox.h"

#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>

namespace glight::gui {

class ShowWindow;

class EffectPropertiesWindow : public PropertiesWindow {
 public:
  EffectPropertiesWindow(theatre::Effect &effect,
                         theatre::Management &management,
                         ShowWindow &parentWindow);

  theatre::FolderObject &GetObject() final override;

 private:
  void fillProperties();
  void fillConnectionsList();

  void onAddConnectionClicked();
  void onRemoveConnectionClicked();
  void onSelectedConnectionChanged();
  void onInputSelected(theatre::SourceValue *preset);
  void onUpdateControllables();

  Gtk::Label _titleLabel;
  Gtk::TreeView _connectionsListView;
  Glib::RefPtr<Gtk::ListStore> _connectionsListModel;
  struct ConnectionsListColumns : public Gtk::TreeModelColumnRecord {
    ConnectionsListColumns() {
      add(_title);
      add(_index);
      add(_inputIndex);
    }

    Gtk::TreeModelColumn<Glib::ustring> _title;
    Gtk::TreeModelColumn<size_t> _index;
    Gtk::TreeModelColumn<size_t> _inputIndex;
  } _connectionsListColumns;

  Gtk::VBox _topBox;
  Gtk::HBox _mainHBox, _connectionsBox;
  Gtk::Frame _connectionsFrame, _propertiesFrame;
  std::unique_ptr<theatre::PropertySet> _propertySet;
  PropertiesBox _propertiesBox;

  Gtk::ScrolledWindow _connectionsScrolledWindow;
  Gtk::VButtonBox _connectionsButtonBox;
  Gtk::Button _addConnectionButton, _removeConnectionButton;

  theatre::Effect *_effect;
  theatre::Management *_management;
  ShowWindow &_parentWindow;
};

}  // namespace glight::gui

#endif
