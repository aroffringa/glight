#ifndef GUI_EFFECT_PROPERTIES_WINDOW_H_
#define GUI_EFFECT_PROPERTIES_WINDOW_H_

#include "propertieswindow.h"

#include "theatre/forwards.h"

#include "gui/scopedconnection.h"
#include "gui/components/propertiesbox.h"

#include <gtkmm/dialog.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>

namespace glight::gui {

class EffectPropertiesWindow : public PropertiesWindow {
 public:
  EffectPropertiesWindow(theatre::Effect &effect);

  theatre::FolderObject &GetObject() final override;

 private:
  void fillProperties();
  void fillConnectionsList();

  void onAddConnectionClicked();
  void onConnectControllableClicked();
  void onRemoveConnectionClicked();
  void onSelectedConnectionChanged();
  void onInputsSelected(const std::vector<theatre::SourceValue *> &sources);
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

  Gtk::Box _topBox{Gtk::Orientation::VERTICAL};
  Gtk::Box _mainHBox, _connectionsBox;
  Gtk::Frame _connectionsFrame{"Connections"};
  Gtk::Frame _propertiesFrame{"Properties"};
  std::unique_ptr<theatre::PropertySet> _propertySet;
  PropertiesBox _propertiesBox;
  ScopedConnection update_connection_;

  Gtk::ScrolledWindow _connectionsScrolledWindow;
  Gtk::Box _connectionsButtonBox;
  Gtk::Button _addConnectionButton{"Add"};
  Gtk::Button _connectControllablesButton{};
  Gtk::Button _removeConnectionButton{"Remove"};

  std::unique_ptr<Gtk::Dialog> dialog_;

  theatre::Effect *_effect;
};

}  // namespace glight::gui

#endif
