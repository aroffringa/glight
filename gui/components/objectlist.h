#ifndef GUI_OBJECT_TREE_H_
#define GUI_OBJECT_TREE_H_

#include <giomm/menu.h>
#include <giomm/simpleactiongroup.h>

#include <gdkmm/pixbuf.h>

#include <gtkmm/iconpaintable.h>
#include <gtkmm/image.h>
#include <gtkmm/liststore.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/popovermenu.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>

#include "../recursionlock.h"

#include "../../theatre/forwards.h"

namespace glight::gui {

using system::ObservingPtr;

enum class ObjectListType {
  AllExceptFixtures,
  All,
  OnlyPresetCollections,
  OnlyChases,
  OnlyEffects,
  OnlyVariables
};

class EventTransmitter;

class ObjectList : public Gtk::ScrolledWindow {
 public:
  ObjectList();

  ObjectListType DisplayType() const { return _displayType; }
  void SetDisplayType(ObjectListType displayType) {
    _displayType = displayType;
    fillList();
  }

  system::ObservingPtr<theatre::FolderObject> SelectedObject() const;

  std::vector<system::ObservingPtr<theatre::FolderObject>> Selection() const;

  sigc::signal<void()> &SignalSelectionChange() {
    return _signalSelectionChange;
  }

  sigc::signal<void(ObservingPtr<theatre::FolderObject> object)>
      &SignalObjectActivated() {
    return _signalObjectActivated;
  }

  void SelectObject(const theatre::FolderObject &object);

  void SetFolder(theatre::Folder &folder) {
    if (&folder != _openFolder) {
      _openFolder = &folder;
      bool doChangeSelection =
          (_listView.get_selection()->count_selected_rows() != 0);
      if (doChangeSelection) _listView.get_selection()->unselect_all();
      fillList();
      if (doChangeSelection) _signalSelectionChange.emit();
    }
  }

  void SetShowTypeColumn(bool showTypeColumn);
  bool ShowTypeColumn() const { return _showTypeColumn; }

  void SetAllowMultiSelection(bool allow_multi_selection) {
    _listView.get_selection()->set_mode(allow_multi_selection
                                            ? Gtk::SelectionMode::MULTIPLE
                                            : Gtk::SelectionMode::SINGLE);
    _listView.set_rubber_banding(allow_multi_selection);
  }

 private:
  enum ObjectListType _displayType = ObjectListType::AllExceptFixtures;
  bool _showFixtureGroups = true;
  bool _showTypeColumn = false;
  theatre::Folder *_openFolder;

  class TreeViewWithMenu : public Gtk::TreeView {
   public:
    TreeViewWithMenu(ObjectList &parent);

   private:
    ObjectList &_parent;
    void OnButtonPress(double x, double y);
  } _listView;

  Glib::RefPtr<Gtk::ListStore> _listModel;
  struct ListColumns : public Gtk::TreeModelColumnRecord {
    ListColumns() {
      add(_icon);
      add(_type);
      add(_title);
      add(_object);
    }

    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> _icon;
    Gtk::TreeModelColumn<Glib::ustring> _type;
    Gtk::TreeModelColumn<Glib::ustring> _title;
    Gtk::TreeModelColumn<system::ObservingPtr<theatre::FolderObject>> _object;
  } _listColumns;

  void fillList();
  void fillListFolder(const theatre::Folder &folder,
                      const theatre::FolderObject *selectedObj);
  bool selectObject(const theatre::FolderObject &object,
                    const Gtk::TreeModel::Children &children);
  void constructContextMenu();
  void constructFolderMenu(const std::shared_ptr<Gio::Menu> &menu,
                           Gio::ActionMap &actions, theatre::Folder &folder,
                           int &counter);
  void onMoveSelected(theatre::Folder *destination);
  void onMoveUpSelected();
  void onMoveDownSelected();

  sigc::signal<void()> _signalSelectionChange;
  sigc::signal<void(system::ObservingPtr<theatre::FolderObject> object)>
      _signalObjectActivated;

  RecursionLock _avoidRecursion;
  Gtk::PopoverMenu _contextMenu;
  std::unique_ptr<Gtk::MessageDialog> dialog_;
};

}  // namespace glight::gui

#endif
